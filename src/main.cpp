
#include <SDL2/SDL.h>
#include <glad.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <flags.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "lib/basic.h"
#include "render.h"
#include "math.h"

#define get(type,name) (args.get<type>(name) ? *args.get<type>(name) : (std::cout << "Failed to get arg " << name << "!" << std::endl, exit(1), type()))

i32 cli_main(i32 argc, char** argv) {
	
	flags::args args(argc, argv);

	i32 w = get(int,"w");
	i32 h = get(int,"h");
	i32 s = get(int,"s");
	std::string o = get(std::string,"o");

	bool region = false;
	i32 x = 0, y = 0, rw = 0, rh = 0;
	if(args.get<int>("x")) {
		x = get(int,"x");
		y = get(int,"y");
		rw = get(int,"rw");
		rh = get(int,"rh");
		region = true;
	}

	std::cout << "Initializing renderer..." << std::endl;

	renderer result;
	result.init(w,h,s,false);
	result.set_region(region, x, y, rw, rh);

	std::cout << "Building scene..." << std::endl;

	scene sc;
	sc.init(w,h);

	std::cout << "Rendering " << w << "x" << h << "x" << s << " to " << o << "..." << std::endl;
	u64 start = result.begin_render(sc);

	while(!result.finish()) {
		std::cout << "Completed: " << result.progress() * 100.0f << "%" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	u64 end = SDL_GetPerformanceCounter();

	std::cout << "Finished in " << (f64)(end - start) / SDL_GetPerformanceFrequency() << "s" << std::endl;
	std::cout << "Writing to file..." << std::endl;
	result.write_to_file(o);
	result.destroy();

	return 0;
}

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

void gui_main() {

	plt_setup();

	ImGui::GetStyle().WindowRounding = 0.0f;

	bool do_region = false;
	i32 size[3] = {640,480,8};
	i32 region[4] = {220,270,150,150};

	u64 time = 0, start = 0;
	std::string file = "output.png";
	file.resize(100);

	scene s;
	renderer result;

	s.init(size[0],size[1]);
	result.init(size[0], size[1], size[2]);

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
		ImGui::Checkbox("##do_region", &do_region);
		ImGui::SameLine();
		ImGui::InputInt4("Region", region);
		ImGui::InputText("##file",(char*)file.c_str(),file.size());
		ImGui::SameLine();
		if(ImGui::Button("Save")) {
			result.write_to_file(file);
		}

		if(ImGui::Button("Generate")) {
			result.destroy();
			s.destroy();

			s.init(size[0], size[1]);
			result.init(size[0], size[1], size[2]);
			result.set_region(do_region, region[0], region[1], region[2], region[3]);

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
}

i32 main(i32 argc, char** argv) {
	
	if(argc > 1) {
		return cli_main(argc, argv);
	}
	gui_main();
	return 0;
}
