#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( int argc, char** argv ){
	/********************
	********************/
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	/********************
	********************/
	int AUDIO_IN_CHS = 2;
	int AUDIO_OUT_CHS = 2;
	
	int soundStream_Input_DeviceId = -1;
	int soundStream_Output_DeviceId = -1;
	
	int CamWidth	= 1920;
	int CamHeight	= 1080;
	int Cam_id = -1;
	int CamFps = 30;

	/********************
	********************/
	printf("---------------------------------\n");
	printf("> parameters\n");
	printf("\t-i      Audio in (-1)\n");
	printf("\t-o      Audio out(-1)\n");
	printf("\t-ichs   Audio in  chs(2)\n");
	printf("\t-ochs   Audio out chs(2)\n");
	printf("\t-c      camera_id(-1)\n");
	printf("\t-w      cam_width(1920)\n");
	printf("\t-h      cam_height(1080)\n");
	printf("\t-CamFps CamFps(30)\n");
	printf("---------------------------------\n");
	
	for(int i = 1; i < argc; i++){
		if( strcmp(argv[i], "-i") == 0 ){
			if(i+1 < argc) { soundStream_Input_DeviceId = atoi(argv[i+1]); i++; }
		}else if( strcmp(argv[i], "-o") == 0 ){
			if(i+1 < argc) { soundStream_Output_DeviceId = atoi(argv[i+1]); i++; }
		}else if( strcmp(argv[i], "-ichs") == 0 ){
			if(i+1 < argc) { AUDIO_IN_CHS = atoi(argv[i+1]); i++; }
		}else if( strcmp(argv[i], "-ochs") == 0 ){
			if(i+1 < argc) { AUDIO_OUT_CHS = atoi(argv[i+1]); i++; }
		}else if( strcmp(argv[i], "-c") == 0 ){
			if(i+1 < argc) Cam_id = atoi(argv[i+1]);
		}else if( strcmp(argv[i], "-w") == 0 ){
			if(i+1 < argc) CamWidth = atoi(argv[i+1]);
		}else if( strcmp(argv[i], "-h") == 0 ){
			if(i+1 < argc) CamHeight = atoi(argv[i+1]);
		}else if( strcmp(argv[i], "-CamFps") == 0 ){
			if(i+1 < argc) CamFps = atoi(argv[i+1]);
		}
	}
	
	/********************
	********************/
	printf("> parameters from main\n");
	printf("sound_In      = %d\n", soundStream_Input_DeviceId);
	printf("sound_Out     = %d\n", soundStream_Output_DeviceId);
	printf("AUDIO_IN_CHS  = %d\n", AUDIO_IN_CHS);
	printf("AUDIO_OUT_CHS = %d\n", AUDIO_OUT_CHS);
	printf("CamWidth      = %d\n", CamWidth);
	printf("CamHeight     = %d\n", CamHeight);
	printf("Cam_id        = %d\n", Cam_id);
	printf("CamFps        = %d\n", CamFps);
	fflush(stdout);
	
	/********************
	comment out for multi-window
	********************/
	ofRunApp(new ofApp( soundStream_Input_DeviceId, soundStream_Output_DeviceId, AUDIO_IN_CHS, AUDIO_OUT_CHS, Cam_id, CamWidth, CamHeight, CamFps ));
}
