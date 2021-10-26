/*
 * VideoShader.cpp
 *
 *  Created on: 25 окт. 2021 г.
 *      Author: pi
 */

#include "VideoShader.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;


VideoShader::VideoShader(const char* file)
{
	int ret;

	ret = load_shader(file);
	if (ret < 0)
	{
		cerr << "failed to create program: " << file << endl;
		m_ok = false;
		return;
	}
	m_program = ret;
	glBindAttribLocation(m_program, 0, "position");
	ret = link_program(m_program);
	if (ret)
	{
		cerr << "failed to link program: " << file << endl;
		m_ok = false;
		return;
	}
	m_ok = true;
}

VideoShader::~VideoShader()
{
	// TODO Auto-generated destructor stub
}


const char VideoShader::fullscreen_vs[] =
		"attribute vec3 position;                \n"
		"void main()                             \n"
		"{                                       \n"
		"  //  gl_Position = vec4(position.x, position.y*0.8, position.z, 1.0);  \n"
		"    gl_Position = vec4(position, 1.0);  \n"
		"}                                       \n";

const char VideoShader::fullscreen_fs_head[] =
		"precision mediump float;    \n"
		"uniform vec3      pDisplay; // viewport resolution in pixels \n"
		"uniform vec3      pFrame;   // video pixels w, h, macro-pixels w, h \n"
		"uniform sampler2D pVideo;   // Video plane #1 \n"
		"uniform sampler2D pVideo_a; // Video plane #2 \n"
		"uniform sampler2D pVideo_b; // Video plane #3 \n"
		"  \n";
const char VideoShader::fullscreen_fs_tail[] =
		"                                                                                     \n"
		"void main()                                                                          \n"
		"{                                                                                    \n"
		"    mainImage(gl_FragColor, gl_FragCoord.xy);                                        \n"
		"}                                                                                    \n";



int VideoShader::create_program(const char *vs_src, const char *fs_src)
{
	GLuint vertex_shader, fragment_shader, program;
	GLint ret;

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertex_shader, 1, &vs_src, NULL);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("vertex shader compilation failed!:\n");
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &ret);
		if (ret > 1) {
			log = (char *)malloc(ret);
			glGetShaderInfoLog(vertex_shader, ret, NULL, log);
			printf("%s", log);
			free(log);
		}

		return -1;
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragment_shader, 1, &fs_src, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("fragment shader compilation failed!:\n");
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1) {
			log = (char *)malloc(ret);
			glGetShaderInfoLog(fragment_shader, ret, NULL, log);
			printf("%s", log);
			free(log);
		}

		return -1;
	}

	program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	return program;
}

int VideoShader::link_program(unsigned program)
{
	GLint ret;

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("program linking failed!:\n");
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1) {
			log = (char *)malloc(ret);
			glGetProgramInfoLog(program, ret, NULL, log);
			printf("%s", log);
			free(log);
		}

		return -1;
	}

	return 0;
}


int VideoShader::load_shader(const char *file)
{
	std::ifstream input;
	std::stringstream text;

	input.open(file);
	text << fullscreen_fs_head << input.rdbuf() << fullscreen_fs_tail;
	input.close();

	return create_program(fullscreen_vs, (const char *)text.str().c_str());
}
