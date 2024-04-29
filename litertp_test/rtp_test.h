#pragma once

#include "mongoose.h"
#include <string>
#include <thread>

#include <litertp/litertp.h>

class rtp_test
{
public:
	bool init();
	void cleanup();

	void require_key_frame();

private:
	void run();

	static void mg_callback(struct mg_connection* c, int ev, void* ev_data, void* fn_data);

protected:

	void on_setoffer(std::string s, int& code, std::string& rsp);
	void on_getoffer(std::string s, int& code, std::string& rsp);
	void on_setanswer(std::string s, int& code, std::string& rsp);

	static void s_litertp_on_frame(void* ctx, uint32_t ssrc, uint16_t pt, int frequency, int channels, const av_frame_t* frame);

	static void s_litertp_on_keyframe_required(void* ctx, uint32_t ssrc, int mode);

private:
	bool active_ = false;
	mg_mgr mgr_ = {0};
	
	std::thread* thread_ = nullptr;
	
	litertp_session_t* session_ = nullptr;
};


