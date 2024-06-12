/**
 * @file rtp_session.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "rtp_session.h"
#include <sys2/util.h>

namespace litertp
{
	rtp_session::rtp_session(bool webrtc):
		timer_(&rtp_session::run,this)
	{
		cname_= sys::util::random_string(20);

		webrtc_ = webrtc;
		if (webrtc_)
		{
			ice_ufrag_ = sys::util::random_string(5);
			ice_pwd_ = sys::util::random_string(25);
			ice_options_ = "trickle";
		}
	}

	rtp_session::~rtp_session()
	{
		close();
	}

	void rtp_session::close()
	{
		active_ = false;
		signal_.notify();
		if (timer_.joinable()) {
			timer_.join();
		}

		clear_media_streams();
		clear_transports();
	}



	media_stream_ptr rtp_session::create_media_stream(media_type_t mt, uint32_t ssrc, const char* local_address, int local_rtp_port, int local_rtcp_port)
	{
		std::unique_lock<std::shared_mutex>lk(streams_mutex_);
		auto itr = streams_.find(mt);
		if (itr != streams_.end())
		{
			return itr->second;
		}

		auto tp = create_udp_transport(local_rtp_port);
		if (!tp)
		{
			return nullptr;
		}
		auto tp2 = create_udp_transport(local_rtcp_port);
		if (!tp2)
		{
			remote_transport(local_rtp_port);
			return nullptr;
		}

		std::string mid = std::to_string((int)mt);
		media_stream_ptr m = std::make_shared<media_stream>(mt, ssrc, mid, cname_,ice_options_, ice_ufrag_, ice_pwd_, local_address, tp, tp2);
		
		m->litertp_on_frame_.add(s_litertp_on_frame, this);
		m->litertp_on_keyframe_required_.add(s_litertp_on_keyframe_required, this);
		m->litertp_on_rtcp_app_.add(s_litertp_on_rtcp_app, this);
		m->litertp_on_rtcp_bye_.add(s_litertp_on_rtcp_bye, this);
		m->litertp_on_rtcp_report_.add(s_litertp_on_rtcp_report, this);

		streams_.insert(std::make_pair(mt, m));

		return m;
	}

	media_stream_ptr rtp_session::create_media_stream(media_type_t mt, uint32_t ssrc, int port, litertp_on_send_packet on_send_packet, void* ctx)
	{
		std::unique_lock<std::shared_mutex>lk(streams_mutex_);
		auto itr = streams_.find(mt);
		if (itr != streams_.end())
		{
			return itr->second;
		}

		auto tp = create_custom_transport(port, on_send_packet, ctx);
		if (!tp)
		{
			return nullptr;
		}
		

		std::string mid = std::to_string((int)mt);
		media_stream_ptr m = std::make_shared<media_stream>(mt, ssrc, mid, cname_,ice_options_, ice_ufrag_, ice_pwd_,"0.0.0.0", tp, tp,true);
		m->litertp_on_frame_.add(s_litertp_on_frame, this);
		m->litertp_on_keyframe_required_.add(s_litertp_on_keyframe_required, this);
		m->litertp_on_rtcp_app_.add(s_litertp_on_rtcp_app, this);
		m->litertp_on_rtcp_bye_.add(s_litertp_on_rtcp_bye, this);
		m->litertp_on_rtcp_report_.add(s_litertp_on_rtcp_report, this);
		streams_.insert(std::make_pair(mt, m));

		return m;
	}

	media_stream_ptr rtp_session::get_media_stream(media_type_t mt)
	{
		std::shared_lock<std::shared_mutex>lk(streams_mutex_);
		auto itr = streams_.find(mt);
		if (itr == streams_.end())
		{
			return nullptr;
		}

		return itr->second;
	}

	std::vector<media_stream_ptr> rtp_session::get_media_streams()
	{
		std::vector<media_stream_ptr> vec;
		std::shared_lock<std::shared_mutex> lk(streams_mutex_);
		if (streams_.size() > 0)
		{
			vec.reserve(streams_.size());
			for (auto stream : streams_)
			{
				vec.push_back(stream.second);
			}
		}
		return vec;
	}

	void rtp_session::remove_media_stream(media_type_t mt)
	{
		std::unique_lock<std::shared_mutex>lk(streams_mutex_);
		streams_.erase(mt);
	}

	void rtp_session::clear_media_streams()
	{
		std::unique_lock<std::shared_mutex>lk(streams_mutex_);
		streams_.clear();
	}


	void rtp_session::clear_local_candidates()
	{
		auto ms = get_media_streams();
		for (auto m : ms)
		{
			m->clear_local_candidates();
		}
	}

	void rtp_session::clear_remote_candidates()
	{
		auto ms = get_media_streams();
		for (auto m : ms)
		{
			m->clear_remote_candidates();
		}
	}

	bool rtp_session::receive_rtp_packet(int port, const uint8_t* rtp_packet, int size)
	{
		auto tp = get_transport(port);
		if (!tp)
		{
			return false;
		}
		return tp->receive_rtp_packet(rtp_packet, size);
	}

	bool rtp_session::receive_rtcp_packet(int port, const uint8_t* rtp_packet, int size)
	{
		auto tp = get_transport(port);
		if (!tp)
		{
			return false;
		}
		return tp->receive_rtcp_packet(rtp_packet, size);
	}

	std::string rtp_session::create_offer()
	{
		litertp::sdp sdp;
		if (webrtc_)
		{
			sdp.attrs_.push_back(sdp_pair("msid-semantic", "WMS", ":"));
		}
		sdp.bundle_ = local_group_bundle();
		auto streams=get_media_streams();
		for (auto stream : streams)
		{
			stream->set_sdp_type(sdp_type_offer);
			auto sdpm = stream->get_local_sdp();
			sdp.medias_.push_back(sdpm);
		}

		return sdp.to_string();
	}

	std::string rtp_session::create_answer()
	{
		litertp::sdp sdp;
		if (webrtc_)
		{
			sdp.attrs_.push_back(sdp_pair("msid-semantic", "WMS", ":"));
		}
		sdp.bundle_ = local_group_bundle();
		auto streams = get_media_streams();
		for (auto stream : streams)
		{
			stream->set_sdp_type(sdp_type_answer);
			if(!stream->negotiate())
			{
				continue;
			}

			auto sdpm_local = stream->get_local_sdp();
			auto sdpm_remote = stream->get_remote_sdp();

			sdpm_remote.rtcp_address_ = sdpm_local.rtcp_address_;
			sdpm_remote.rtcp_address_type_ = sdpm_local.rtcp_address_type_;
			sdpm_remote.rtcp_port_ = sdpm_local.rtcp_port_;
			sdpm_remote.rtcp_mux_ = sdpm_local.rtcp_mux_;
			sdpm_remote.rtp_address_ = sdpm_local.rtp_address_;
			sdpm_remote.rtp_address_type_ = sdpm_local.rtp_address_type_;
			sdpm_remote.rtp_network_type_ = sdpm_local.rtp_network_type_;
			sdpm_remote.rtp_port_ = sdpm_local.rtp_port_;

			sdpm_remote.candidates_ = sdpm_local.candidates_;
			sdpm_remote.ice_ufrag_ = sdpm_local.ice_ufrag_;
			sdpm_remote.ice_pwd_ = sdpm_local.ice_pwd_;
			sdpm_remote.ice_options_ = sdpm_local.ice_options_;
			sdpm_remote.fingerprint_sign_ = sdpm_local.fingerprint_sign_;
			sdpm_remote.fingerprint_ = sdpm_local.fingerprint_;
			sdpm_remote.setup_ = sdpm_local.setup_;
			sdpm_remote.trans_mode_ = sdpm_local.trans_mode_;
			sdpm_remote.ssrcs_ = sdpm_local.ssrcs_;
			sdpm_remote.ssrc_group_ = sdpm_local.ssrc_group_;

			sdp.medias_.push_back(sdpm_remote);
		}


		return sdp.to_string();
	}

	bool rtp_session::set_remote_sdp(const char* sdp, sdp_type_t sdp_type)
	{
		litertp::sdp sdpo;
		if (!sdpo.parse(sdp))
		{
			return false;
		}

		for (auto& m : sdpo.medias_)
		{
			auto s=get_media_stream(m.media_type_);
			if (s)
			{
				if (!s->set_remote_sdp(m, sdp_type))
				{
					return false;
				}
			}
		}

		return true;
	}

	bool rtp_session::set_local_sdp(const char* sdp)
	{
		litertp::sdp s;
		if (!s.parse(sdp))
		{
			return false;
		}

		clear_transports();
		clear_media_streams();

		for (auto& m : s.medias_)
		{
			auto s = create_media_stream(m.media_type_,m.get_default_ssrc(), m.rtp_address_.c_str(), m.rtp_port_, m.rtcp_port_);
			if (!s)
			{
				clear_transports();
				clear_media_streams();
				return false;
			}
			s->set_local_sdp(m);
		}

		return true;
	}

	void rtp_session::require_keyframe()
	{
		auto streams = get_media_streams();
		for (auto stream : streams)
		{
			if (stream->media_type() == media_type_video)
			{
				uint32_t ssrc_media=stream->get_remote_ssrc();
				stream->send_rtcp_keyframe(ssrc_media);
				
			}
		}
	}


	transport_ptr rtp_session::create_udp_transport(int port)
	{
		std::unique_lock<std::shared_mutex>lk(transports_mutex_);

		auto itr = transports_.find(port);
		if (itr != transports_.end())
		{
			transport_udp* p = dynamic_cast<transport_udp*>(itr->second.get());
			if (!p)
			{
				return nullptr;
			}
			return itr->second;
		}

		transport_ptr tp = std::make_shared<transport_udp>(port);
		if (!tp->start())
		{
			return nullptr;
		}

		tp->ice_ufrag_local_ = this->ice_ufrag_;
		tp->ice_pwd_local_ = this->ice_pwd_;

		transports_.insert(std::make_pair(port, tp));
		return tp;
	}

	transport_ptr rtp_session::create_custom_transport(int port, litertp_on_send_packet on_send_packet, void* ctx)
	{
		std::unique_lock<std::shared_mutex>lk(transports_mutex_);

		auto itr = transports_.find(port);
		if (itr != transports_.end())
		{
			transport_custom* p = dynamic_cast<transport_custom*>(itr->second.get());
			if (!p)
			{
				return nullptr;
			}
			return itr->second;
		}

		transport_ptr tp = std::make_shared<transport_custom>(port, on_send_packet, ctx);
		if (!tp->start())
		{
			return nullptr;
		}

		transports_.insert(std::make_pair(port, tp));
		return tp;
	}

	transport_ptr rtp_session::get_transport(int port)
	{
		std::shared_lock<std::shared_mutex>lk(transports_mutex_);

		auto itr = transports_.find(port);
		if (itr == transports_.end())
		{
			return nullptr;
		}

		return itr->second;
	}

	void rtp_session::remote_transport(int port)
	{
		std::unique_lock<std::shared_mutex>lk(transports_mutex_);
		transports_.erase(port);
	}

	void rtp_session::clear_transports()
	{
		std::unique_lock<std::shared_mutex>lk(transports_mutex_);
		transports_.clear();
	}



	void rtp_session::s_litertp_on_frame(void* ctx, uint32_t ssrc, uint16_t pt, int frequency, int channels, const av_frame_t* frame)
	{
		rtp_session* p = (rtp_session*)ctx;
		p->litertp_on_frame_.invoke(ssrc, pt, frequency, channels, frame);
	}

	void rtp_session::s_litertp_on_keyframe_required(void* ctx, uint32_t ssrc, int mode)
	{
		rtp_session* p = (rtp_session*)ctx;
		p->litertp_on_keyframe_required_.invoke(ssrc,mode);

	}

	void rtp_session::s_litertp_on_rtcp_bye(void* ctx, uint32_t* ssrcs, int ssrc_count, const char* message)
	{
		rtp_session* p = (rtp_session*)ctx;
		p->litertp_on_rtcp_bye_.invoke(ssrcs, ssrc_count, message);
	}

	void rtp_session::s_litertp_on_rtcp_app(void* ctx, uint32_t ssrc, uint32_t name, const char* appdata, uint32_t data_size)
	{
		rtp_session* p = (rtp_session*)ctx;
		p->litertp_on_rtcp_app_.invoke(ssrc, name,appdata,data_size);
	}

	void rtp_session::s_litertp_on_rtcp_report(void* ctx, uint32_t ssrc)
	{
		rtp_session* p = (rtp_session*)ctx;
		p->litertp_on_rtcp_report_.invoke(ssrc);
	}

	void rtp_session::run()
	{
		while (active_)
		{
			auto ms = get_media_streams();
			for (auto m : ms)
			{
				m->run_rtcp_stats();
			}

			signal_.wait(5000);
		}
	}

	bool rtp_session::local_group_bundle()
	{
		auto ms = get_media_streams();
		auto itr_begin = ms.begin();
		if (itr_begin == ms.end())
		{
			return false;
		}
		auto sdpm_begin = (*itr_begin)->get_local_sdp();
		for (auto itr = ms.begin(); itr != ms.end(); itr++)
		{
			auto sdpm = (*itr)->get_local_sdp();
			if (sdpm_begin.rtp_port_ != sdpm.rtp_port_)
			{
				return false;
			}
		}

		return true;
	}
}

