/************************************************************
************************************************************/
#include <ofApp.h>

/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp(int _soundStream_Input_DeviceId, int _soundStream_Output_DeviceId, int _AUDIO_IN_CHS, int _AUDIO_OUT_CHS, int _Cam_id, int _CamWidth, int _CamHeight, int _CamFps)
: b_EnableAudioOut(false)
, soundStream_Input_DeviceId(_soundStream_Input_DeviceId)
, soundStream_Output_DeviceId(_soundStream_Output_DeviceId)
, AUDIO_IN_CHS(_AUDIO_IN_CHS)
, AUDIO_OUT_CHS(_AUDIO_OUT_CHS)
, Cam_id(_Cam_id)
, CamWidth(_CamWidth)
, CamHeight(_CamHeight)
, CamFps(_CamFps)
{
	/********************
	********************/
	if( (AUDIO_IN_CHS != 1) || (AUDIO_IN_CHS != 2) )	{ AUDIO_IN_CHS = 2; }
	if( (AUDIO_OUT_CHS != 1) || (AUDIO_OUT_CHS != 2) )	{ AUDIO_OUT_CHS = 2; }
}

/******************************
******************************/
ofApp::~ofApp()
{
	delete Gui_Global;
	
	if(fp_Log)	fclose(fp_Log);
}


/******************************
******************************/
void ofApp::exit(){
	soundStream->close();
	delete soundStream;
	
	printf("> Good-bye\n");
	fflush(stdout);
}

/******************************
******************************/
void ofApp::setup(){
	/********************
	********************/
	ofSetBackgroundAuto(true);
	
	ofSetWindowTitle("RippleShader");
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	ofSetWindowShape(MAIN_WINDOW_W, MAIN_WINDOW_H);
	ofSetEscapeQuitsApp(false);
	
	/********************
	********************/
	ofSeedRandom();
	
	/********************
	********************/
	fp_Log = fopen("../../../data/Log.csv", "w");
	
	setup_Gui();
	
	fbo_out.allocate(MAIN_WINDOW_W, MAIN_WINDOW_H, GL_RGBA, 4);
	ClearFbo(fbo_out);
	
	img_Frame.load("img_frame/Frame.png");
	
	/********************
	********************/
	if(AUDIO_IN_CHS == 2)	AudioFFT.set_AnalyzeCh(AUDIO_FFT::ANALYZE_CH__STEREO);
	else					AudioFFT.set_AnalyzeCh(AUDIO_FFT::ANALYZE_CH__L);
	
	update_AudioFFT_ParamForFFTCal();
	AudioFFT.setup(AUDIO_BUF_SIZE, AUDIO_SAMPLERATE);
	
	/********************
	print device list : both sound and camera
	********************/
	soundStream = new ofSoundStream();
	
	print_separatoin();
	printf("> List of Sound Devices\n");
	soundStream->printDeviceList();
	
	printf("> List of webCam Devices\n");
	print_webCam_listDevices();
	
	/********************
	********************/
	print_separatoin();
	printf("> sound Device\n");
	fflush(stdout);
	
	/*
	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = 44100;
	settings.numInputChannels = 2;
	settings.numOutputChannels = 2;
	settings.bufferSize = bufferSize;
	
	soundStream.setup(settings);
	*/
	/* */
	if( soundStream_Input_DeviceId == -1 ){
		ofExit();
		return;
		
	}else{
		/********************
			soundStream->setup()
		の直後、audioIn()/audioOut()がstartする.
		これらのmethodは、AudioFFTにaccessするので、この前に、AudioFFTが初期化されていないと、不正accessが発生してしまう.
		********************/
		setup_sound();
	}
	
	/********************
	********************/
	DrawFFT.setup(AudioFFT);
	DrawArtSin.setup(AudioFFT);
	
	/********************
	********************/
	print_separatoin();
	printf("> setup camera\n");
	fflush(stdout);
	
	setup_camera();
	// fbo_WebCam.allocate(WebCam.getWidth(), WebCam.getHeight(), GL_RGBA);
	fbo_WebCam.allocate(MAIN_WINDOW_W, MAIN_WINDOW_H, GL_RGBA);
	ClearFbo(fbo_WebCam);
	
	shader_Ripple.load( "sj_shader/Ripple.vert", "sj_shader/Ripple.frag" );
	shader_Gray.load( "sj_shader/Gray.vert", "sj_shader/Gray.frag" );
	
	/********************
	Set whether or not the aspect ratio of this camera is forced to a non-default setting.
	The camera's aspect ratio, by default, is the aspect ratio of your viewport. If you have set a non-default value (with ofCamera::setAspectRatio()), you can toggle whether or not this value is applied.
	********************/
	// camera.setForceAspectRatio(true);
	
	// set_CamParam();
	
	/********************
	********************/
	printf("> start process\n");
	fflush(stdout);
}

/******************************
description
	memoryを確保は、app start後にしないと、
	segmentation faultになってしまった。
******************************/
void ofApp::setup_Gui()
{
	/********************
	********************/
	Gui_Global = new GUI_GLOBAL;
	Gui_Global->setup("param", "gui.xml", 10, 10);
}

/******************************
******************************/
void ofApp::print_webCam_listDevices()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	WebCam.setVerbose(true);
	
	vector< ofVideoDevice > Devices = WebCam.listDevices();// 上 2行がないと、List表示されない.
}

/******************************
******************************/
void ofApp::setup_camera()
{
	/********************
	細かく情報出してもらう
	********************/
	ofSetLogLevel(OF_LOG_VERBOSE);
    WebCam.setVerbose(false); // here, already checked the list.
	
	vector< ofVideoDevice > Devices = WebCam.listDevices();// 上 2行がないと、List表示されない.
	
	/********************
	********************/
	if(Cam_id == -1){
		ofExit();
		return;
	}else{
		if(Devices.size() <= Cam_id) { printf("size = %d\n", Devices.size()); ERROR_MSG(); ofExit(); return; }
		
		WebCam.setDeviceID(Cam_id);
		printf("\n> WebCam set device id = %d\n", Cam_id);
		printf("> DeviceID     = %d\n", Devices[Cam_id].id);
		printf("> CamName      = %s\n", Devices[Cam_id].deviceName.c_str());
		printf("> HardwareName = %s\n", Devices[Cam_id].hardwareName.c_str());
		printf("> serialID     = %s\n", Devices[Cam_id].serialID.c_str());
		
		printf( "\n> Cam size asked  = (%d, %d)\n", int(CamWidth), int(CamHeight) );
		/*
			https://openframeworks.cc/documentation/video/ofVideoGrabber/#show_setDesiredFrameRate
				Set's the desired frame rate of the grabber. This should be called before initGrabber(), which will try to initialize at the desired frame rate. 
				Not all frame rates will be supported, but this at least gives you some abilitity to try grab at different rates.
		*/
		WebCam.setDesiredFrameRate(CamFps);
		// WebCam.initGrabber(CAM_WIDTH, CAM_HEIGHT);
		WebCam.setup(CamWidth, CamHeight, true); // bool ofVideoGrabber::initGrabber(int w, int h, bool setUseTexture){
		printf( "> Cam size actual = (%d, %d)\n\n", int(WebCam.getWidth()), int(WebCam.getHeight()) );
		fflush(stdout);
	}
}

/******************************
******************************/
void ofApp::setup_sound(){
	/********************
	********************/
	vector<ofSoundDevice> devices = soundStream->getDeviceList();
	
	ofSoundStreamSettings settings;
	
	settings.setInDevice(devices[soundStream_Input_DeviceId]);
	if(devices[soundStream_Input_DeviceId].name.find("Native Instruments") != std::string::npos){
		/********************
		"name" でDeviceを選択する場合は、
		このblockを参考にしてください。
		********************/
		printf("> name match : Native Instruments selected\n");
		fflush(stdout);
	}
	
	if( soundStream_Output_DeviceId != -1 ){
		settings.setOutDevice(devices[soundStream_Output_DeviceId]);
		b_EnableAudioOut = true;
	}

	settings.setInListener(this);
	settings.numInputChannels = AUDIO_IN_CHS;
	
	if(b_EnableAudioOut){
		settings.setOutListener(this); /* Don't forget this */
		settings.numOutputChannels = AUDIO_OUT_CHS;
	}else{
		settings.numOutputChannels = 0;
	}
	
	settings.sampleRate = AUDIO_SAMPLERATE;
	settings.bufferSize = AUDIO_BUF_SIZE;
	settings.numBuffers = AUDIO_BUFFERS; // 使ってないっぽい
	
	/********************
		soundStream->setup()
	の直後、audioIn()/audioOut()がstartする.
	これらのmethodは、AudioFFTにaccessするので、この前に、AudioFFTが初期化されていないと、不正accessが発生してしまう.
	********************/
	soundStream->setup(settings);
}

/******************************
******************************/
void ofApp::update_AudioFFT_ParamForFFTCal(){
	
	AudioFFT.update_ParamForFFTCal(	Gui_Global->FFT__SoftGain,
									Gui_Global->FFT__k_smooth,
									Gui_Global->FFT__dt_smooth_2_N,
									Gui_Global->FFT__dt_smooth_2_A,
									Gui_Global->FFT__b_HanningWindow,
									Gui_Global->FFT__Afilter_0dB_at_Hz,
									Gui_Global->ArtSin_Band_min__N,
									Gui_Global->ArtSin_Band_max__N,
									Gui_Global->ArtSin_Band_min__A,
									Gui_Global->ArtSin_Band_max__A,
									Gui_Global->ArtSin_PhaseMap_k,
									Gui_Global->b_ArtSin_PhaseMap_MoreDynamic_N,
									Gui_Global->b_ArtSin_PhaseMap_MoreDynamic_A,
									Gui_Global->b_ArtSin_abs,
									Gui_Global->b_TukeyWindow_artSin,
									Gui_Global->Tukey_alpha);
	
}

/******************************
******************************/
void ofApp::update(){
	/********************
	********************/
	if(Gui_Global->b_Audio_Start)	{ Gui_Global->b_Audio_Start = false; Sound__Start(); }
	if(Gui_Global->b_Audio_Stop)	{ Gui_Global->b_Audio_Stop = false; Sound__Stop(); }
	if(Gui_Global->b_Audio_Reset)	{ Gui_Global->b_Audio_Reset = false; Sound__Reset(); }
	
	/********************
	********************/
	update_AudioFFT_ParamForFFTCal();
	AudioFFT.update();
	
	DrawFFT.update(AudioFFT);
	DrawArtSin.update(AudioFFT);
	
	/********************
	********************/
	update_spectrum_image();
	
	/********************
	********************/
	WebCam.update();
	if(WebCam.isFrameNew()) {
		fbo_WebCam.begin();
			if(Gui_Global->b_WebCam_Gray)	shader_Gray.begin();
			
			ofPushMatrix();
			if(Gui_Global->b_WebCam_Mirror){
				ofTranslate(fbo_WebCam.getWidth(), 0);
				ofScale(-1, 1, 1);
			}
				
				ofBackground(0, 0, 0, 255);
				ofSetColor(255);
				WebCam.draw(0, 0, fbo_WebCam.getWidth(), fbo_WebCam.getHeight());
				
			ofPopMatrix();
			
			if(Gui_Global->b_WebCam_Gray)	shader_Gray.end();
		fbo_WebCam.end();
	}
	
}

/******************************
******************************/
void ofApp::update_spectrum_image(){
	for(int i = 0; i < AUDIO_BUF_SIZE/2; i++){
		switch( int(Gui_Global->Ripple_source) ){
			case 0:
				spectrum[i] = AudioFFT.get_GainSmoothed_N(i) * Gui_Global->Ripple_Gain;
				break;
			case 1:
				spectrum[i] = AudioFFT.get_GainSmoothed2_N(i) * Gui_Global->Ripple_Gain;
				break;
			case 2:
				spectrum[i] = AudioFFT.get_GainSmoothed_A(i) * Gui_Global->Ripple_Gain;
				break;
			case 3:
				spectrum[i] = AudioFFT.get_GainSmoothed2_A(i) * Gui_Global->Ripple_Gain;
				break;
				
			case 4:
				spectrum[i] = AudioFFT.get_artSin_N(AUDIO_BUF_SIZE/2 - 1 - i) * Gui_Global->Ripple_Gain;
				break;
			case 5:
				spectrum[i] = AudioFFT.get_artSin_A(AUDIO_BUF_SIZE/2 - 1 - i) * Gui_Global->Ripple_Gain;
				break;
			case 6:
				spectrum[i] = AudioFFT.get_artSin_MixDown(AUDIO_BUF_SIZE/2 - 1 - i) * Gui_Global->Ripple_Gain;
				break;
			default:
				spectrum[i] = AudioFFT.get_GainSmoothed2_N(i) * Gui_Global->Ripple_Gain;
				break;
		}
	}
	
	spectrumImage.setFromPixels( spectrum, AUDIO_BUF_SIZE/2, 1, OF_IMAGE_GRAYSCALE );
}

/******************************
******************************/
void ofApp::draw(){
	/********************
	********************/
	ClearFbo(fbo_out);
	
	/********************
	********************/
	if(Gui_Global->b_DispWebCam){
		draw_fboWebCam(fbo_out);
	}else{
		draw_frame(fbo_out);
		DrawFFT.draw(fbo_out, AudioFFT);
		DrawArtSin.draw(fbo_out);
	}
	
	/********************
	********************/
	ofEnableAntiAliasing();
	ofEnableBlendMode(OF_BLENDMODE_DISABLED); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
	
	ofBackground(0, 0, 0, 255);
	ofSetColor(255, 255, 255, 255);
	
	fbo_out.draw(0, 0);
	
	/********************
	********************/
	if(Gui_Global->b_Disp){
		/* */
		Gui_Global->gui.draw();
		
		/* */
		ofSetColor(255);
		string info;
		info += "FPS = " + ofToString(ofGetFrameRate(), 2) + "\n";
		ofDrawBitmapString(info, 30, 30);
		
		/* */
		draw_RippleSource_info();
	}
}

/******************************
******************************/
void ofApp::draw_RippleSource_info(){
	ofSetColor(255);
	string info = "RippleSource =  ";
	
	switch( int(Gui_Global->Ripple_source) ){
		case 0:
			info += "N : GainSmoothed";
			break;
		case 1:
			info += "N : GainSmoothed2";
			break;
		case 2:
			info += "A : GainSmoothed";
			break;
		case 3:
			info += "A : GainSmoothed2";
			break;
		case 4:
			info += "N : ArtSin";
			break;
		case 5:
			info += "A : ArtSin";
			break;
		case 6:
			info += "MixDown : ArtSin";
			break;
		default:
			info += "N : GainSmoothed2";
			break;
	}
	
	ofDrawBitmapString(info, 250, 30);
}

/******************************
******************************/
void ofApp::draw_fboWebCam(ofFbo& fbo){
	fbo.begin();
		/********************
		********************/
		if(Gui_Global->b_UseRippleShader_toWebCam){
			shader_Ripple.begin();
			
			shader_Ripple.setUniform1f( "texture_w", fbo_WebCam.getWidth() );
			shader_Ripple.setUniform1f( "texture_h", fbo_WebCam.getHeight() );
			shader_Ripple.setUniform1f( "space", Gui_Global->Ripple_Space );
			
			shader_Ripple.setUniformTexture( "texture1", spectrumImage.getTexture(), 1 );
		}
			
			/********************
			********************/
			ofEnableAntiAliasing();
			ofEnableBlendMode(OF_BLENDMODE_ALPHA); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
			
			ofSetColor(255);
			fbo_WebCam.draw(0, 0, fbo.getWidth(), fbo.getHeight());
			
		/********************
		********************/
		if(Gui_Global->b_UseRippleShader_toWebCam){
			shader_Ripple.end();
		}
	fbo.end();
}

/******************************
******************************/
void ofApp::draw_frame(ofFbo& fbo){
	fbo.begin();
		ofEnableAntiAliasing();
		ofEnableBlendMode(OF_BLENDMODE_ADD); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
		
		ofSetColor(255);
		img_Frame.draw(0, 0, fbo.getWidth(), fbo.getHeight());
	fbo.end();
}

/******************************
******************************/
void ofApp::audioIn(ofSoundBuffer & buffer){
	AudioFFT.SetVol(buffer);
}

/******************************
******************************/
void ofApp::audioOut(ofSoundBuffer & buffer){
	AudioFFT.GetVol(buffer, b_EnableAudioOut);
}

/******************************
******************************/
void ofApp::Sound__Start(){
	soundStream->start();
	printf("> soundStream : start\n");
	fflush(stdout);
}

/******************************
******************************/
void ofApp::Sound__Stop(){
	soundStream->stop();
	printf("> soundStream : stop\n");
	fflush(stdout);
}

/******************************
******************************/
void ofApp::Sound__Reset(){
	soundStream->close();
	delete soundStream;
	
	soundStream = new ofSoundStream();
	setup_sound();
	
	printf("> soundStream : close->restart\n");
	fflush(stdout);
}

/******************************
******************************/
void ofApp::keyPressed(int key){
	switch(key){
		case 'd':
			Gui_Global->b_Disp = !Gui_Global->b_Disp;
			break;
			
		case 'm':
			Gui_Global->gui.minimizeAll();
			break;
	}
}

/******************************
******************************/
void ofApp::keyReleased(int key){
}

/******************************
******************************/
void ofApp::mouseMoved(int x, int y ){

}

/******************************
******************************/
void ofApp::mouseDragged(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mousePressed(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mouseReleased(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mouseEntered(int x, int y){

}

/******************************
******************************/
void ofApp::mouseExited(int x, int y){

}

/******************************
******************************/
void ofApp::windowResized(int w, int h){

}

/******************************
******************************/
void ofApp::gotMessage(ofMessage msg){

}

/******************************
******************************/
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
