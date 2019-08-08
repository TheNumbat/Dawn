
#include <SDL2/SDL.h>
#include <glad.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

#define USE_THREADING
#define LANE_WIDTH 8

#include "basic.h"
#include "image.h"
#include "math.h"

SDL_Window* window = nullptr;
SDL_GLContext gl_context = nullptr;

void plt_setup() {

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow("Dawn", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 900, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
int main(int, char**) {

	plt_setup();

	ImGui::GetStyle().WindowRounding = 0.0f;

	i32 size[3] = {640,480,8};
	u64 time = 0, start = 0;
	std::string file = "output.png";
	file.resize(100);

	scene s;
	image result;

	s.init(size[0],size[1],size[2]);
	result.init(size[0], size[1]);

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
	    ImGui::InputInt3("Size", size);
	    ImGui::InputText("##file",(char*)file.c_str(),file.size());
	    ImGui::SameLine();
	    if(ImGui::Button("Save")) {
	    	result.write_to_file(file);
	    }

	    if(ImGui::Button("Generate")) {
	    	s.destroy();
	    	result.destroy();

	    	s.init(size[0],size[1],size[2]);
			result.init(size[0], size[1]);

	    	start = result.begin_render(s);
	    }
    	ImGui::SameLine();
	    if(result.in_progress()) {
	    	ImGui::ProgressBar(result.progress());
	    } else {
	    	ImGui::Text("Time: %.3fms", 1000.0f * (f64)time / SDL_GetPerformanceFrequency());
	    }
	    if(result.finish()) {
	    	u64 end = SDL_GetPerformanceCounter();
	    	time = end - start;
	    }

	    ImGui::Image((ImTextureID)(iptr)result.handle, {(f32)size[0],(f32)size[1]});

	    ImGui::End();

		render_frame();
	}

	plt_shutdown();

	return 0;
}
