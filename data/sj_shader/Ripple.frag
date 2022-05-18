/************************************************************
************************************************************/
//Specify compiler to use GLSL version 1.2
//Enable working with textures of any dimensions
//Declare texture texture0, which is linked when you use fbo.bind(), or any other texture.bind().

//#version 430 compatibility
#version 120
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DRect texture0;     //it can be named in any way, GLSL just links it
uniform sampler2DRect texture1;		//spectrum

uniform float texture_w = 1920;
uniform float texture_h =1080;
uniform float space = 7.0;

// #define N (256)
// uniform float specArray[N];

/************************************************************
************************************************************/

/******************************
******************************/
void main(){
	/********************
	********************/
    vec2 pos_0 = gl_TexCoord[0].st;
	
	vec2 center = vec2(texture_w, texture_h) / 2.0;
	float dist = distance( center, pos_0 );	//built-in function for distance 
	
	/* */
	vec2 spectrumPos = vec2( dist / space, 0.5 );
	vec4 color1 =  texture2DRect(texture1, spectrumPos);
	float spectrValue = color1.r;
	
	/********************
	********************/
	vec2 delta = pos_0 - center;
	delta *= 1-2*spectrValue;
	vec2 pos_1 = center + delta;
	
	vec4 color = texture2DRect(texture0, pos_1);
	
	gl_FragColor = color * gl_Color;
}
