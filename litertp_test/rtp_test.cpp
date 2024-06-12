
#include "rtp_test.h"
#include <json/reader.h>
#include <json/writer.h>

bool rtp_test::init()
{
	if (active_) {
		return false;
	}
	active_ = true;


	mg_mgr_init(&mgr_);
	mg_http_listen(&mgr_, "http://0.0.0.0:8001", mg_callback, this);

	thread_ = new std::thread(&rtp_test::run, this);
	return true;
}

void rtp_test::cleanup()
{
	active_ = false;
	if (thread_)
	{
		thread_->join();
		delete thread_;
		thread_ = nullptr;
	}
	mg_mgr_free(&mgr_);

	if (session_)
	{
		litertp_destory_session(&session_);
	}
}

void rtp_test::require_key_frame()
{
	
}

void rtp_test::run()
{
	while (active_)
	{
		mg_mgr_poll(&mgr_, 1000);
	}
}

void rtp_test::mg_callback(struct mg_connection* c, int ev, void* ev_data, void* fn_data)
{
	rtp_test* p = (rtp_test*)fn_data;

	if (ev == MG_EV_HTTP_MSG) {
		struct mg_http_message* hm = (mg_http_message*)ev_data, tmp = { 0 };

		int code = 200;
		std::string rsp;
		if (mg_http_match_uri(hm, "/set-offer"))
		{
			p->on_setoffer(std::string(hm->body.ptr, hm->body.len), code,rsp);
		}
		else if (mg_http_match_uri(hm, "/get-offer"))
		{
			p->on_getoffer(std::string(hm->body.ptr, hm->body.len), code, rsp);
		}
		else if (mg_http_match_uri(hm, "/set-answer"))
		{
			p->on_setanswer(std::string(hm->body.ptr, hm->body.len), code, rsp);
		}
		std::string headers;
		headers.append("Access-Control-Allow-Origin: *\r\n");
		headers.append("Access-Control-Allow-Headers: *\r\n");
			
		mg_http_reply(c, 200, headers.c_str(), rsp.c_str());
	}
}


void rtp_test::on_setoffer(std::string s, int& code, std::string& rsp)
{
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(s, root)) {
		code = 400;
		rsp = "Bad request";
		return;
	}

	
	std::string sOffer = root["offerSdp"].asCString();
	


	session_ = litertp_create_session(1);

	litertp_set_on_frame_eventhandler(session_, s_litertp_on_frame, this);
	litertp_set_on_keyframe_required_eventhandler(session_, s_litertp_on_keyframe_required, this);


	litertp_create_media_stream(session_,media_type_audio,0, rtp_trans_mode_sendrecv,true, "0.0.0.0", 50000, 50000);
	litertp_add_local_audio_track(session_,codec_type_opus, 96, 48000, 2);

	litertp_create_media_stream(session_, media_type_video, 0, rtp_trans_mode_sendrecv, true, "0.0.0.0", 50000, 50000);
	litertp_add_local_video_track(session_, codec_type_vp8, 126, 90000);
	litertp_add_local_video_track(session_, codec_type_h264, 125, 90000);

	litertp_add_local_candidate(session_,media_type_audio, 1, 1, "192.168.0.85", 50000, 0x7e7c1eff);
	litertp_add_local_candidate(session_, media_type_video, 2, 1, "192.168.0.86", 50000, 0x7e7c1eff);

	litertp_set_remote_sdp(session_, sOffer.c_str(),sdp_type_offer);

	char* local_sdp = nullptr;
	int sdp_size = 0;
	int r=litertp_create_answer(session_, &local_sdp, &sdp_size);



	Json::Value jdata;
	jdata["answerSdp"] = local_sdp;

	Json::Value jrsp;
	jrsp["code"] = code;
	jrsp["data"] = jdata;

	Json::FastWriter w;
	rsp = w.write(jrsp);

	litertp_free((void**)&local_sdp);


	

}

void rtp_test::on_getoffer(std::string s, int& code, std::string& rsp)
{
	session_ = litertp_create_session(1);

	litertp_set_on_frame_eventhandler(session_, s_litertp_on_frame, this);
	litertp_set_on_keyframe_required_eventhandler(session_, s_litertp_on_keyframe_required, this);

	litertp_create_media_stream(session_, media_type_audio, 0, rtp_trans_mode_recvonly, true, "0.0.0.0", 50000, 50000);
	litertp_add_local_audio_track(session_,  codec_type_opus, 96, 48000, 2);

	litertp_create_media_stream(session_, media_type_video, 0, rtp_trans_mode_recvonly,true, "0.0.0.0", 50000, 50000);
	litertp_add_local_video_track(session_, codec_type_vp8, 125, 90000);

	litertp_add_local_candidate(session_, media_type_audio, 1, 1, "192.168.0.85", 50000, 2122260223);
	litertp_add_local_candidate(session_, media_type_video, 2, 1, "192.168.0.85", 50000, 2122260223);
	//litertp_add_local_candate(session_, media_type_video,2, 2, "192.168.0.62", 50002, 2122260223);

	char* local_sdp = nullptr;
	int sdp_size = 0;
	litertp_create_offer(session_, &local_sdp, &sdp_size);


	Json::Value jdata;
	jdata["offerSdp"] = local_sdp;

	Json::Value jrsp;
	jrsp["code"] = code;
	jrsp["data"] = jdata;

	Json::FastWriter w;
	rsp = w.write(jrsp);

	litertp_free((void**)&local_sdp);

}

void rtp_test::on_setanswer(std::string s, int& code, std::string& rsp)
{
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(s, root)) {
		code = 400;
		rsp = "Bad request";
		return;
	}

	std::string sAnswer = root["answerSdp"].asCString();

	if (session_)
	{
		litertp_set_remote_sdp(session_, sAnswer.c_str(), sdp_type_answer);
	}
}

void rtp_test::s_litertp_on_frame(void* ctx, uint32_t ssrc, uint16_t pt, int frequency, int channels, const av_frame_t* frame)
{
	rtp_test* p = (rtp_test*)ctx;
	printf("rtp frame mt=%d size=%d\n",frame->mt,frame->data_size);
}

void rtp_test::s_litertp_on_keyframe_required(void* ctx, uint32_t ssrc, int mode)
{
	rtp_test* p = (rtp_test*)ctx;

}



