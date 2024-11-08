/**
* Author: Seha Kim
* Assignment: Lunar Lander
* Date due: 2024-10-26, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <vector>
#include "Entity.h"

#include <ctime>
#include "cmath"

enum AppStatus { RUNNING, TERMINATED };

struct GameState
{
    Entity* ship;
    Entity* map;
    Entity* platforms;
    Entity* target;
};

GameState g_state;

enum Coordinate
{
    x_coordinate,
    y_coordinate
};

//enum Direction { LEFT, RIGHT, UP, DOWN };

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 500 * 2,
WINDOW_HEIGHT = 400 * 2;


const float BG_RED = 0.9608f,
BG_BLUE = 0.9608f,
BG_GREEN = 0.9608f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
//const float DEGREES_PER_SECOND = 90.0f;

const glm::vec3 ORIGIN = glm::vec3(0.0f, 0.0f, 0.0f),
DOUBLE = glm::vec3(2.0f, 2.0f, 0.0f);

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

//TIMESTEP
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
constexpr float ACC_OF_GRAVITY = -1.5f;
constexpr int PLATFORM_COUNT = 5;
float g_time_accumulator = 0.0f;


const char SHIP_SPRITE_FILEPATH[] = "sprites/spaceship_spritesheet.png";
const char MAP_SPRITE_FILEPATH[] = "sprites/lunar_platform.png";
const char TARGET_SPRITE_FILEPATH[] = "sprites/target_platform.png";
const char FONT_SPRITE_FILEPATH[] = "sprites/font1.png";

const int FONTBANK_SIZE = 16,
FRAMES_PER_SECOND = 4;

SDL_Window* g_display_window;
bool g_game_is_running = true;
bool g_is_growing = true;

bool failed = false;
bool succeed = false;
const int map_speed = 1.0f;

ShaderProgram g_shader_program;

glm::mat4 g_view_matrix,
g_ship_model_matrix,
//g_frame_model_matrix,
g_projection_matrix;

float g_previous_ticks = 0.0f;

GLuint g_ship_texture_id,
g_map_texture_id,
g_win_texture_id,
g_target_texture_id,
//g_frame_texture_id,
g_font_texture_id;

float g_ship_speed = 1.0f;

bool is_moving_h = false;
bool is_moving_v = false;
constexpr int SPRITESHEET_DIMENSIONS_COLUMNS = 4;
constexpr int SPIRTESHEET_DIMENSIONS_ROWS = 2;



bool s_key = false;
bool w_key = false;
bool a_key = false;
bool d_key = false;

float g_animation_time = 0.0f;
int g_animation_frames = 2;
int g_animation_index = 0;


void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
   
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        int spritesheet_index = (int)text[i];  
        float offset = (screen_size + spacing) * i;

        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index, int rows, int cols)
{
    float u_coord = (float)(index % cols) / (float)cols;
    float v_coord = (float)(index / cols) / (float)rows;

    float width = 1.0f / (float)cols;
    float height = 1.0f / (float)rows;

    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Wrong Image.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return textureID;
}

void initialise()
{


    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    g_display_window = SDL_CreateWindow("Hello, Animation!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);  
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  


    g_ship_model_matrix = glm::mat4(1.0f);
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    g_ship_texture_id = load_texture(SHIP_SPRITE_FILEPATH);
    g_map_texture_id = load_texture(MAP_SPRITE_FILEPATH);
    g_target_texture_id = load_texture(TARGET_SPRITE_FILEPATH);
    g_font_texture_id = load_texture(FONT_SPRITE_FILEPATH);


    int g_ship_flying[4][2] =
    {
        {0, 4}, 
        {1, 5}, 
        {2, 6}, 
        {3, 7}  
    };

    g_state.ship = new Entity(g_ship_texture_id, g_ship_speed, g_ship_flying, g_animation_time, g_animation_frames,
        g_animation_index, SPRITESHEET_DIMENSIONS_COLUMNS, SPIRTESHEET_DIMENSIONS_ROWS);

    g_state.ship->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    g_state.ship->set_position(glm::vec3(0.0f, 3.0f, 0.0f));
    g_state.platforms = new Entity[PLATFORM_COUNT];

    g_state.target = new Entity(g_target_texture_id, 0);

    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        g_state.platforms[i].set_texture_id(load_texture(MAP_SPRITE_FILEPATH));
        g_state.platforms[i].set_position(glm::vec3(i - 1.0f, -2.8f, 0.0f));
        g_state.platforms[i].update(0.0f, nullptr, 0, false, false);
    }

    g_state.map = new Entity(g_map_texture_id, map_speed);
    g_state.ship->face_down();

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_state.ship->set_movement(glm::vec3(0.0f));

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_WINDOWEVENT_CLOSE:
        case SDL_QUIT:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                g_game_is_running = false;
                break;

            default:
                break;
            }
        default:
            break;
        }
    }
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_state.ship->move_left();
        a_key = true;
        is_moving_h = true;
    }

    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_state.ship->move_right();
        d_key = true;
        is_moving_h = true;
    }
    else
    {
        is_moving_h = false;
    }

    if (key_state[SDL_SCANCODE_UP])
    {
        g_state.ship->move_up();
        w_key = true;
        is_moving_v = true;
    }

    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_state.ship->move_down();
        s_key = true;
        is_moving_v = true;
    }
    else { is_moving_v = false; }



    if (glm::length(g_state.ship->get_movement()) > 1.0f)
    {
        g_state.ship->normalise_movement();
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; 
    float delta_time = ticks - g_previous_ticks; 
    g_previous_ticks = ticks;


    delta_time += g_time_accumulator;


    if (delta_time < FIXED_TIMESTEP)
    {
        g_time_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_state.ship->update(FIXED_TIMESTEP, g_state.platforms, PLATFORM_COUNT, is_moving_h, is_moving_v);
        g_state.target->update(FIXED_TIMESTEP);
        for (int i = 0; i < PLATFORM_COUNT; i++) {
            g_state.platforms[i].update(FIXED_TIMESTEP);
            g_state.platforms[i].set_scale(glm::vec3(10.0f, 0.5f, 1.0f));
        }
        delta_time -= FIXED_TIMESTEP;

    }

    if (g_state.ship->check_collision(g_state.target))
    {
        g_state.ship->set_velocity(glm::vec3(0.0f, 0.0f, 0.0f));
        succeed = true;
    }
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        if (g_state.ship->check_collision(&g_state.platforms[i]))
        {
            failed = true;
        }
    }


    g_state.target->set_position(glm::vec3(-1.0f, -2.4f, 0.0f));
    g_state.target->set_scale(glm::vec3(0.8f, 0.8f, 0.8f));
    g_state.ship->set_scale(glm::vec3(0.5f, 0.5f, 0.5f));


}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    g_state.ship->render(&g_shader_program);
    g_state.target->render(&g_shader_program);
    for (int i = 0; i < PLATFORM_COUNT; i++)
        g_state.platforms[i].render(&g_shader_program);

    if (succeed == true)
    {
        draw_text(&g_shader_program, g_font_texture_id, "MISSION SUCCESSFUL", 0.2f, 0.005f, glm::vec3(-3.5f, 0.0f, 0.0f));
    }

    if (failed == true)
    {
        draw_text(&g_shader_program, g_font_texture_id, "MISSION FAILED", 0.2f, 0.005f, glm::vec3(-2.5f, 0.0f, 0.0f));
    }


    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
