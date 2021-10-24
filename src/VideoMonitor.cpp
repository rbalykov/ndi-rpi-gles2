
#include <VideoMonitor.h>
#include <Log.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>

//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <err.h>
//#include <sys/mman.h>
//#include <string.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


using namespace std;


VideoMonitor::VideoMonitor():
		sCino (VideoStub ("extras/625x799.uyvy", 625, 799)),
		sPattern("extras/2560x1440.uyvy", 2560, 1440),
		sIndian("extras/789x444.uyvy", 789, 444),
		sBlack("extras/black_2x1.uyvy", 2, 1)
{
}

VideoMonitor::~VideoMonitor()
{
}

int VideoMonitor::Run()
{

	return 0;
}

void VideoMonitor::Update()
{
//	receiver.Discover();
}

void VideoMonitor::Render()
{


}
void VideoMonitor::Cleanup()
{

}

void VideoMonitor::Draw(uint32_t frame_id)
{
//	glUniform1f(iTime, (get_time_ns() - start_time) / (double) NSEC_PER_SEC);
	// Replace the above to input elapsed time relative to 60 FPS
	// glUniform1f(iTime, (float) frame / 60.0f);
	glUniform1ui(iFrame, frame_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 1);
	glUniform1i(pVideo, 0);
	glUniform1f(pPixels, 625.0);
	glUniform1f(pPairs, 313.0);


//	start_perfcntrs();

	glDrawArrays(GL_TRIANGLES, 0, 6);

//	end_perfcntrs();
}


bool VideoMonitor::Init(GBM_Instance &gbm, const EGL_Instance &egl)
{
	int ret;
	GLuint program, vbo;
	GLint iResolution;
	void *texture;

	ret = load_shader(shader_file);
	if (ret < 0) {
		printf("failed to create program\n");
		return -1;
	}

	program = ret;

	ret = link_program(program);
	if (ret) {
		printf("failed to link program\n");
		return -1;
	}
	//	texture = load_test("extras/625x799.uyvy");
	//	texture = load_test("extras/2560x1440.uyvy");
	//	texture = load_test("extras/789x444.uyvy");


	glViewport(0, 0, gbm.w(), gbm.h());
	glUseProgram(program);
	iTime = glGetUniformLocation(program, "iTime");
	iFrame = glGetUniformLocation(program, "iFrame");
	iResolution = glGetUniformLocation(program, "iResolution");
	pPixels = glGetUniformLocation(program, "pPixels");
	pPairs = glGetUniformLocation(program, "pPairs");
	pVideo = glGetUniformLocation(program, "pVideo");

	glUniform3f(iResolution, gbm.w(), gbm.h(), 0);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) (intptr_t) 0);
	glEnableVertexAttribArray(0);

	return true;
}

bool VideoMonitor::init_shadertoy(const char *file)
{
	int ret = load_shader(file);
	stoy_program = ret;

	glBindAttribLocation(program, 0, "position");

	ret = link_program(stoy_program);

	glUseProgram(stoy_program);
	stoy_time_loc = glGetUniformLocation(stoy_program, "iTime");

	/* we can set iResolution a single time, it doesn't change: */
	GLint resolution_location = glGetUniformLocation(stoy_program, "iResolution");
	glUniform3f(resolution_location, texw, texh, 0);

	glGenFramebuffers(1, &stoy_fbo);
	glGenTextures(1, &stoy_fbotex);
	glBindFramebuffer(GL_FRAMEBUFFER, stoy_fbo);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, stoy_fbotex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texw, texh, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		stoy_fbotex, 0);

	const GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};
	glGenBuffers(1, &stoy_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, stoy_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)0);

	return 0;
}


int VideoMonitor::create_program(const char *vs_src, const char *fs_src)
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

int VideoMonitor::link_program(unsigned program)
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


int VideoMonitor::load_shader(const char *file)
{
	ifstream input;
	stringstream text;

	input.open(file);
	text << fullscreen_fs_head << input.rdbuf() << fullscreen_fs_tail;
	input.close();

	return create_program(fullscreen_vs, (const char *)text.str().c_str());
}



/*
int VideoMonitor::load_shader(const char *file)
{
	struct stat statbuf;
	char *frag;
	int fd, ret;


	fd = open(file, 0);
	if (fd < 0) {
		err(fd, "could not open '%s'", file);
	}

	ret = fstat(fd, &statbuf);
	if (ret < 0) {
		err(ret, "could not stat '%s'", file);
	}

	const char *text =
		(char *) mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	asprintf(&frag, fullscreen_fs, text);

	return create_program(fullscreen_vs, frag);
}
*/

const GLfloat VideoMonitor::vertices[12] =
{
	// First triangle:
	1.0f, 1.0f,
	-1.0f, 1.0f,
	-1.0f, -1.0f,
	// Second triangle:
	-1.0f, -1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f,
};

const GLfloat VideoMonitor::vVertices[] =
{
	-1.0f, -1.0f, 0.0f,
	+1.0f, -1.0f, 0.0f,
	-1.0f, +1.0f, 0.0f,
	+1.0f, +1.0f, 0.0f
};

const GLfloat VideoMonitor::vTexCoords[] =
{
	1.0f, 1.0f,
	0.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f
};

const GLfloat VideoMonitor::vNormals[] =
{
	+0.0f, +0.0f, +1.0f, // forward
	+0.0f, +0.0f, +1.0f, // forward
	+0.0f, +0.0f, +1.0f, // forward
	+0.0f, +0.0f, +1.0f, // forward
};

const char VideoMonitor::shader_file[] = "extras/convert.glsl";

const char VideoMonitor::scene_vs[] =
	"uniform mat4 modelviewMatrix;      \n"
	"uniform mat4 modelviewprojectionMatrix;\n"
	"uniform mat3 normalMatrix;         \n"
	"                                   \n"
	"attribute vec4 in_position;        \n"
	"attribute vec3 in_normal;          \n"
	"attribute vec2 in_TexCoord;        \n"
	"                                   \n"
	"vec4 lightSource = vec4(2.0, 2.0, 20.0, 0.0);\n"
	"                                   \n"
	"varying vec4 vVaryingColor;        \n"
	"varying vec2 vTexCoord;            \n"
	"                                   \n"
	"void main()                        \n"
	"{                                  \n"
	"    gl_Position = modelviewprojectionMatrix * in_position;\n"
	"    vec3 vEyeNormal = normalMatrix * in_normal;\n"
	"    vec4 vPosition4 = modelviewMatrix * in_position;\n"
	"    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;\n"
	"    vec3 vLightDir = normalize(lightSource.xyz - vPosition3);\n"
	"    float diff = max(0.0, dot(vEyeNormal, vLightDir));\n"
	"    vVaryingColor = vec4(diff * vec3(1.0, 1.0, 1.0), 1.0);\n"
	"    vTexCoord = in_TexCoord; \n"
	"}                            \n";

const char VideoMonitor::scene_fs[] =
	"precision mediump float;           \n"
	"                                   \n"
	"uniform sampler2D uTex;            \n"
	"                                   \n"
	"varying vec4 vVaryingColor;        \n"
	"varying vec2 vTexCoord;            \n"
	"                                   \n"
	"void main()                        \n"
	"{                                  \n"
	"    gl_FragColor = vVaryingColor * texture2D(uTex, vTexCoord);\n"
	"}                                  \n";

const char VideoMonitor::transcode_vs[] =
	"attribute vec3 position;           \n"
	"void main()                        \n"
	"{                                  \n"
	"    gl_Position = vec4(position, 1.0);\n"
	"}                                  \n";

//	const char transcode_fs_head[] =
const char VideoMonitor::transcode_fs_impl[] =
	"precision mediump float;                                                             \n"
	"uniform vec3      iResolution;           // viewport resolution (in pixels)          \n"
	"uniform float     iGlobalTime;           // shader playback time (in seconds)        \n"
	"uniform vec4      iMouse;                // mouse pixel coords                       \n"
	"uniform vec4      iDate;                 // (year, month, day, time in seconds)      \n"
	"uniform float     iSampleRate;           // sound sample rate (i.e., 44100)          \n"
	"uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)           \n"
	"uniform float     iChannelTime[4];       // channel playback time (in sec)           \n"
	"uniform float     iTime;                                                             \n"
//		"                                                                                     \n";
//	const char transcode_fs_tail[] =
	"                                                                                     \n"
	"void main()                                                                          \n"
	"{                                                                                    \n"
	"    mainImage(gl_FragColor, gl_FragCoord.xy);                                        \n"
	"}                                                                                    \n";


const char VideoMonitor::fullscreen_vs[] =
		"attribute vec3 position;                \n"
		"void main()                             \n"
		"{                                       \n"
		"    gl_Position = vec4(position.x, position.y*0.8, position.z, 1.0);  \n"
		"}                                       \n";

const char VideoMonitor::fullscreen_fs_head[] =
		"precision mediump float;                                                             \n"
		"uniform vec3      iResolution;           // viewport resolution (in pixels)          \n"
		"uniform float     iTime;                 // shader playback time (in seconds)        \n"
		"uniform int       iFrame;                // current frame number                     \n"
		"uniform float pPixels; \n"
		"uniform float pPairs;  \n"
		"uniform sampler2D pVideo; \n"
		"                                                                                     \n"
	;//	"%s                                                                                   \n"
const char VideoMonitor::fullscreen_fs_tail[] =
		"                                                                                     \n"
		"void main()                                                                          \n"
		"{                                                                                    \n"
		"    mainImage(gl_FragColor, gl_FragCoord.xy);                                        \n"
		"}                                                                                    \n";


VideoStub::VideoStub(const char * file, GLuint _w, GLuint _h)
{
	h = _h; w = _w; pairs = (w+1)/2;
    std::ifstream input( file, std::ios::binary );
    if (!input.good())
    {
    	cerr << "can't open file: " << file << endl;
    	m_ok = false;
    	return;
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
    if (buffer.size() != pairs * h * 4)
    {
    	std::cerr << "wrong stub data for file " << file
    			<<", expected "<<  pairs * h * 4
    			<< ", got " << buffer.size() << endl;
    	m_ok = false;
    	return;
    }

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pairs, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	m_ok = true;
};

VideoStub::~VideoStub()
{}

