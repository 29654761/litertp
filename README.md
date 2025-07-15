# litertp
This project is obsolete, Please see https://github.com/29654761/cxx-deps/tree/main/rtpx

### Introduce

After I failed to compile the webrtc source code, I tried to find other similar rtp transfer libraries, but unfortunately, they were either written too simply, without weak network adversarial mechanisms, or too strongly associated with their own projects. Finally, I decided to try writing my own.

I called my project litertp.  As it's name "lite", this project only implement rtp network transport. Even without negotiation.

You can use this lib with your projects such as **webrtc**, **rtsp**, **sip**, **h323** and others.

Litertp implements **nack**,**fir**,**pli**, not support fec,rtx,transport-cc.

### Build

The source code based on C++17.

This project uses the cmake build system. 

##### CMake Options

```
LITERTP_SSL=ON/OFF         # Build with openssl and srtp
LITERTP_SHARED=ON/OFF      # Build with shared libaray
LITERTP_STATIC=ON/OFF      # Build with static libaray
OPENSSL_INCLUDE=path/to/openssl/include   # if LITERTP_SSL is ON, this must be set
OPENSSL_LIB_CRYPTO=path/to/libcrypto.lib  # if LITERTP_SSL is ON, this must be set
OPENSSL_LIB_SSL=path/to/libssl.lib        # if LITERTP_SSL is ON, this must be set
SRTP_INCLUDE=path/to/srtp/include         # if LITERTP_SSL is ON, this must be set
SRTP_LIB=path/to/libsrtp2.lib             # if LITERTP_SSL is ON, this must be set
```



##### Build for Visual Studio

    set ABI=x64|win32
    
    cmake . ^
    -G "Visual Studio 17 2022" ^
    -A %ABI% ^
    -DLITERTP_SSL=ON ^
    -DLITERTP_SHARED=ON ^
    -DLITERTP_STATIC=ON ^
    -DOPENSSL_INCLUDE="path/to/openssl/include" ^
    -DOPENSSL_LIB_CRYPTO="path/to/libcrypto.lib" ^
    -DOPENSSL_LIB_SSL="path/to/libssl.lib" ^
    -DSRTP_INCLUDE="path/to/srtp/include" ^
    -DSRTP_LIB="path/to/libsrtp2.lib"


​    

##### Build for OSX

```
ABI=MAC|MAC_ARM64|MAC_UNIVERSAL|OS64|OS64COMBINED

cmake . \
-G "Xcode"
-DCMAKE_CXX_FLAGS="--std=c++17" \
-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} \
-DPLATFORM=${ABI} \
-DLITERTP_SSL=ON \
-DLITERTP_SHARED=OFF \
-DOPENSSL_INCLUDE="path/to/openssl/include" \
-DOPENSSL_LIB_CRYPTO="path/to/libcrypto.lib" \
-DOPENSSL_LIB_SSL="path/to/libssl.lib" \
-DSRTP_INCLUDE="path/to/srtp/include" \
-DSRTP_LIB="path/to/libsrtp2.lib"
```

CMake for osx need a config file, I recommended use this project

[https://github.com/leetal/ios-cmake](ios-cmake)



##### Build for Android

```
ABI=armeabi-v7a,arm64-v8a

cmake . \
-B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} \
-DCMAKE_CXX_FLAGS="--std=c++17" \
-DANDROID_ABI=${ABI} \
-DANDROID_PLATFORM=android-23 \
-DLITERTP_SSL=ON \
-DOPENSSL_INCLUDE="path/to/openssl/include" \
-DOPENSSL_LIB_CRYPTO="path/to/libcrypto.lib" \
-DOPENSSL_LIB_SSL="path/to/libssl.lib" \
-DSRTP_INCLUDE="path/to/srtp/include" \
-DSRTP_LIB="path/to/libsrtp2.lib"
```



##### Scripts

I have prepared some scripts in source root path, you can edit slightly to build

```
./build-android.sh armeabie-v7a
./build-osx.sh MAC_UNIVERSAL
./build-win.bat win32
```



### Usage

##### Initialization

First of all, you must initialize the library

```c++
litertp_global_init()
```

And if no longer need, cleanup the library

```c++
litertp_global_cleanup()
```

##### Create Offer

```c++
//create session
litertp_session_t session=litertp_create_session(1); // paramter 1 indicates webrtc rtp

//set event callback
litertp_set_on_frame_eventhandler(session,onframe,ctx);
litertp_set_on_keyframe_required_eventhandler(session,on_keyframe_required,ctx);

//set audio capabilities
litertp_create_media_stream(session,media_type_audio,0,rtp_trans_mode_sendrecv,true,"0.0.0.0",50000,50000);
litertp_add_local_audio_track(session,codec_type_opus,111,48000,2);
litertp_add_local_audio_track(session,codec_type_pcma,8,8000,1);

//set video capabilities
litertp_create_media_stream(session,media_type_video,0,rtp_trans_mode_sendrecv,true,"0.0.0.0",50000,50000);
litertp_add_local_video_track(session,codec_type_h264,96,90000);
litertp_add_local_video_track(session,codec_type_vp8,97,90000);
litertp_add_local_attribute(session,media_type_video,96,"fmtp","level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f");

//if webrtc,you can add candidates, this only set into sdp, Not going to discover
litertp_add_local_candidate(session,media_type_audio,1,1,"192.168.0.11",50000,2122260223);
litertp_add_local_candidate(session,media_type_video,2,1,"192.168.0.11",50000,2122260223);


//generate sdp to send you remote end
char* local_sdp=nullptr;
int sdp_size=0;
litertp_create_offer(session,&local_sdp,&sdp_size);
litertp_free(&local_sdp)

```

##### Set answer

After get remote sdp via your way.

```c++
litertp_set_remote_sdp(session,sdp,sdp_type_answer);


```



##### Create Answer

```c++
//create session
litertp_session_t session=litertp_create_session(1); // paramter 1 indicates webrtc rtp

//set event callback
litertp_set_on_frame_eventhandler(session,onframe,ctx);
litertp_set_on_keyframe_required_eventhandler(session,on_keyframe_required,ctx);

//set audio capabilities
litertp_create_media_stream(session,media_type_audio,0,rtp_trans_mode_sendrecv,true,"0.0.0.0",50000,50000);
litertp_add_local_audio_track(session,codec_type_opus,111,48000,2);
litertp_add_local_audio_track(session,codec_type_pcma,8,8000,1);

//set video capabilities
litertp_create_media_stream(session,media_type_video,0,rtp_trans_mode_sendrecv,true,"0.0.0.0",50000,50000);
litertp_add_local_video_track(session,codec_type_h264,96,90000);
litertp_add_local_video_track(session,codec_type_vp8,97,90000);
litertp_add_local_attribute(session,media_type_video,96,"fmtp","level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f");

//if webrtc,you can add candidates, this only set into sdp, Not going to discover
litertp_add_local_candidate(session,media_type_audio,1,1,"192.168.0.11",50000,2122260223);
litertp_add_local_candidate(session,media_type_video,2,1,"192.168.0.11",50000,2122260223);

//set remote offer sdp to negotiate
litertp_set_remote_sdp(session,sdp,sdp_type_offer);

//create answer
char* local_sdp=nullptr;
int sdp_size=0;
litertp_create_answer(session,&local_sdp,&sdp_size);
litertp_free(&local_sdp)

```



##### SDP

You also can set local sdp for initialize local streams.

```c++
litertp_set_local_sdp(session,sdp)
```

This is equivalent to 

```c++
//set audio capabilities
litertp_create_media_stream(session,media_type_audio,0,rtp_trans_mode_sendrecv,true,"0.0.0.0",50000,50000);
litertp_add_local_audio_track(session,codec_type_opus,111,48000,2);
litertp_add_local_audio_track(session,codec_type_pcma,8,8000,1);

//set video capabilities
litertp_create_media_stream(session,media_type_video,0,rtp_trans_mode_sendrecv,true,"0.0.0.0",50000,50000);
litertp_add_local_video_track(session,codec_type_h264,96,90000);
litertp_add_local_video_track(session,codec_type_vp8,97,90000);
litertp_add_local_attribute(session,media_type_video,96,"fmtp","level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f");

//if webrtc,you can add candidates, this only set into sdp, Not going to discover
litertp_add_local_candidate(session,media_type_audio,1,1,"192.168.0.11",50000,2122260223);
litertp_add_local_candidate(session,media_type_video,2,1,"192.168.0.11",50000,2122260223);
```





And some transport protocol like h.323, you won't get remote sdp, so you can not call `litertp_set_remote_sdp`

 in this case you need to manually construct remote streams

```c++

//set audio capabilities
litertp_create_media_stream
litertp_add_remote_audio_track
litertp_add_remote_audio_track

//set video capabilities
litertp_create_media_stream
litertp_add_remote_video_track
litertp_add_remote_video_track
litertp_add_remote_attribute

    
litertp_set_remote_ssrc
litertp_set_remote_trans_mode
litertp_set_remote_mid
litertp_set_remote_setup

litertp_set_remote_rtp_endpoint
litertp_set_remote_rtcp_endpoint
    
    
```

This is equivalent to 

```
litertp_set_remote_sdp
```



##### Custom transport

Example for supporting `rtsp over tcp`

```C++
//create session
litertp_session_t session=litertp_create_session(0);  // paramter 0 indicates normal rtp

//set event callback
litertp_set_on_frame_eventhandler(session,onframe,ctx);
litertp_set_on_keyframe_required_eventhandler(session,on_keyframe_required,ctx);

//set audio capabilities
litertp_create_media_stream_custom_transport(session,media_type_audio,0,rtp_trans_mode_sendrecv,false,
                                             0,on_send_packet,nullptr);
litertp_add_local_audio_track(session,codec_type_opus,111,48000,2);
litertp_add_local_audio_track(session,codec_type_pcma,8,8000,1);

//set video capabilities
litertp_create_media_stream_custom_transport(session,media_type_video,0,rtp_trans_mode_sendrecv,false,
                                             1,on_send_packet,nullptr);
litertp_add_local_video_track(session,codec_type_h264,96,90000);
litertp_add_local_video_track(session,codec_type_vp8,97,90000);
litertp_add_local_attribute(session,media_type_video,96,"fmtp","level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f");
```

The difference of `litertp_create_media_stream_custom_transport` function is on parameters `port, on_send_packet and ctx` . Custom_transport can not send packet inside, so when it needs to send rtp/rtcp packet, on_send_packet  will be called back, you need to send by youself

```c++
void on_send_packet(void* ctx, int port, int channel, const uint8_t* data, int size)
{
    // magic $
	char magic = '$';
	socket_->send(&magic, 1);

    // channel
	if (port == 0) //audio
	{
		if (channel == 0) //rtp
		{
			socket_->send(&audio_rtp_channel_, 1);
		}
		else if (channel == 1) //rtcp
		{
			socket_->send(&audio_rtcp_channel_, 1);
		}
	}
	else if (port == 1) //video
	{
		if (channel == 0)//rtp
		{
			socket_->send(&video_rtp_channel_, 1);
		}
		else if (channel == 1)//rtcp
		{
			socket_->send(&video_rtcp_channel_, 1);
		}
	}

    // length
	uint16_t len = (uint16_t)size;
	len=sys::socket::hton16(len);
	socket_->send((const char*)&len, 2);
    
    // data
	socket_->send((const char*)data, size);

}
```

For receiving data, you need to receive data and separate out packet bytes by youself,  Then call

```
litertp_receive_rtp_packet(session_,port,rtp_data,size)
litertp_receive_rtcp_packet(session_,port,rtcp_data,size)
```

This will trigger onframe and other rtcp events like normal transport receive data.

##### Go

Now you can send and receive media frame.

The received frames will raised from `litertp_on_frame` set by `litertp_set_on_frame_eventhandler`

To send frame call `litertp_send_frame`



##### Rtcp stats

You can timing call `litertp_get_stats` to get rtcp stats info,  Details to see struct rtp_stats_t
