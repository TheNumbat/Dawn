
#include <SDL2/SDL.h>
#include <glad.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include "basic.h"

#pragma warning(disable : 4201)
#include "math.h"

SDL_Window* window = nullptr;
SDL_GLContext gl_context = nullptr;

void plt_setup() {

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow("Dawn", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);

	gladLoadGL();

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init();

	ImGui::StyleColorsDark();
}

void plt_shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	window = nullptr;
	gl_context = nullptr;
	SDL_Quit();	
}

void render_frame() {

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}

void begin_frame() {

	glClearColor(0.6f, 0.65f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
}

struct image {
	
	image() {}
	static image make(u32 w, u32 h) {
		return image(w,h);
	}
	~image() {
		delete[] _data;
		_data = null;
		if(_handle) glDeleteTextures(1, &_handle);
		_width = _height = _handle = 0;
		_valid = _committed = false;
	}
	void operator=(image&& other) {
		this->~image();
		_valid = other._valid;
		_width = other._width;
		_height = other._height;
		_data = other._data;
		_handle = other._handle;
		other._width = other._height = other._handle = 0;
		other._valid = other._committed = false;
		other._data = null;
	}

	void commit() {
		assert(_valid);
		glBindTexture(GL_TEXTURE_2D, _handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		_committed = true;
	}
	GLuint handle() {
		assert(_valid && _committed);
		return _handle;
	}

	void render() {
		assert(_valid);
		u32* pixel = _data;
		for(u32 y = 0; y < _height; y++) {
			for(u32 x = 0; x < _width; x++) {

				u32 r = (u32)(255.0f * x / _width);
				u32 g = (u32)(255.0f * y / _height);

				(*pixel++) = 0xff000000 | (g << 8) | r;
			}
		}
	}

private:
	bool _valid = false, _committed = false;
	u32 _width = 0, _height = 0;
	u32* _data = nullptr;
	GLuint _handle = 0;

	image(u32 w, u32 h) : _width(w), _height(h), _data(new u32[_width*_height]()), _valid(true) {
		glGenTextures(1, &_handle);
	}
};

int main(int, char**) {

	plt_setup();

	ImGui::GetStyle().WindowRounding = 0.0f;

	iv2 size = {640,480};
	image result = image::make(size.x, size.y);
	
	result.render();
	result.commit();

	bool running = true;
	while(running) {
	
		begin_frame();
		ImGuiIO& io = ImGui::GetIO();

		SDL_Event e;
		while(SDL_PollEvent(&e)) {

			ImGui_ImplSDL2_ProcessEvent(&e);

			switch(e.type) {
			case SDL_QUIT: {
				running = false;
			} break;
			case SDL_KEYDOWN: {
				if(e.key.keysym.sym == SDLK_ESCAPE) {
					running = false;
				}
			} break;
			}
		}

		ImGui::SetNextWindowPos({0,0});
		ImGui::SetNextWindowSize(io.DisplaySize);
		ImGui::Begin("toplevel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
										  ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | 
										  ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    	ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);
	    ImGui::InputInt2("Size",size.a);

	    if(ImGui::Button("Regenerate")) {
	    	result = image::make(size.x, size.y);
	    	result.render();
	    	result.commit();
	    }

	    ImGui::Image((ImTextureID)(iptr)result.handle(), {(f32)size.x,(f32)size.y});

	    ImGui::End();

		render_frame();
	}

	plt_shutdown();

	return 0;
}
