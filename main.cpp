/**
* Author: Seha Kim
* Assignment: Pong Clone
* Date due: 2024-10-12, 11:58pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };
enum ScaleDirection { GROWING, SHRINKING };

constexpr int WINDOW_WIDTH  = 534 * 2,
              WINDOW_HEIGHT = 400 * 2;

constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
float g_previous_ticks = 0.0f;

constexpr float ROT_INCREMENT = 1.0f;

constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL    = 0,
                TEXTURE_BORDER     = 0;
int g_frame_counter = 0;

constexpr char baseballbat1_SPRITE_FILEPATH[]    = "baseballbat1.jpg";
constexpr char baseballbat2_SPRITE_FILEPATH[]    = "baseballbat2.jpg";
constexpr char baseball_SPRITE_FILEPATH[]    = "baseball.jpg";
constexpr char YOUWIN_SPRITE_FILEPATH[]    = "youwin.jpg";

constexpr glm::vec3 INIT_baseballbat1_SCALE = glm::vec3(1.0f, 3.0f, 0.0f), 
INIT_baseballbat2_SCALE = glm::vec3(1.0f, 3.0f, 0.0f), 
INIT_baseball_SCALE = glm::vec3(1.0f, 1.0f, 0.0f);

constexpr glm::vec3 INIT_POS_baseballbat1 = glm::vec3(-4.0f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_POS_baseballbat2 = glm::vec3(4.0f, 0.0f, 0.0f);

bool game_start = false;

bool baseballbat1_collision_top = false;
bool baseballbat1_collision_bottom = false;
bool baseballbat1_collision_top_ai = false;
bool baseballbat1_collision_bottom_ai = false;

bool baseballbat2_collision_top = false;
bool baseballbat2_collision_bottom = false;

bool baseballbat1_youwin = false;
bool baseballbat2_youwin = false;


bool baseball_collision_top = false;
bool baseball_collision_bottom = false;
bool baseball_collision_right = false;
bool baseball_collision_left = false;


bool ai_mode = false;

constexpr float MIN_COLLISION_DISTANCE = 1.0f;

glm::vec3 g_baseballbat2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_baseballbat2_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_baseballbat1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_baseballbat1_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_baseball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_baseball_movement = glm::vec3(1.0f, 1.0f, 0.0f);

float baseball_increment_x = 2.5f;
float baseball_increment_y = 2.5f;
float increment = 2.5f;
float direction = 0.0f;
float g_baseballbat2_speed = 2.5f;
float g_baseballbat1_speed = 2.5f;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

// GLuint initialize textures
glm::mat4 g_view_matrix,
g_baseballbat1_matrix,
g_baseballbat2_matrix,
g_baseball_matrix,
g_youwin_matrix,
g_projection_matrix;

GLuint g_baseballbat1_texture_id;
GLuint g_baseballbat2_texture_id;
GLuint g_baseball_texture_id;
GLuint g_youwin_texture_id;
GLuint g_background_texture_id;

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Two Baseball Bats",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);

    }

#ifdef _WINDOWS
    glewInit();
#endif
  
  glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_baseballbat1_matrix = glm::mat4(1.0f);
    g_baseballbat2_matrix = glm::mat4(1.0f);
    g_baseball_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::mat4(1.0f);
    g_youwin_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());


    g_baseballbat1_texture_id = load_texture(BASEBALLBAT1_SPRITE_FILEPATH);
    g_baseballbat2_texture_id = load_texture(BASEBALLBAT2_SPRITE_FILEPATH);
    g_baseball_texture_id = load_texture(BASEBALL_SPRITE_FILEPATH);
    g_youwin_texture_id = load_texture(YOUWIN_SPRITE_FILEPATH);
    g_background_texture_id = load_texture("baseballstadium.jpg");



    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



void process_input() {
    g_baseballbat1_movement = glm::vec3(0.0f);
    g_baseballbat2_movement = glm::vec3(0.0f);
    g_baseball_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;


        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                g_baseballbat2_movement.y = 1.0f;
                break;

            case SDLK_DOWN:
                g_baseballbat2_movement.y = -1.0f;
                break;

            case SDLK_q:
                g_app_status = TERMINATED;
                break;

            case SDLK_w:
                g_baseballbat1_movement.y = 1.0f;
                break;
            case SDLK_s:
                g_baseballbat1_movement.y = -1.0f;
                break;
            case SDLK_t:
                ai_mode = !(ai_mode);
                break;
            case SDLK_SPACE:
                game_start = true;
                break;

            default:
                break;
            }
        default:
            break;
        }
    }


    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_UP])
    {
        if (baseballbat2_collision_top == false)
        {
            g_baseballbat2_movement.y = 1.0f;
        }
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        if (baseballbat2_collision_bottom == false)
        {
            g_baseballbat2_movement.y = -1.0f;
        }
    }

    if (key_state[SDL_SCANCODE_W])
    {
        if (ai_mode == false)
        {
            if (baseballbat1_collision_top == false)
            {
                g_baseballbat1_movement.y = 1.0f;
            }

        }


    }
    else if (key_state[SDL_SCANCODE_S])
    {
        if (ai_mode == false)
        {
            if (baseballbat1_collision_bottom == false)
            {
                g_baseballbat1_movement.y = -1.0f;
            }

        }

    }

    if (glm::length(g_baseballbat2_movement) > 1.0f)
    {
        g_baseballbat2_movement = glm::normalize(g_baseballbat2_movement);
    }
    if (glm::length(g_baseballbat1_movement) > 1.0f)
    {
        g_baseballbat1_movement = glm::normalize(g_baseballbat1_movement);
    }

}


void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    g_baseballbat2_position += g_baseballbat2_movement * g_baseballbat2_speed * delta_time;
    g_baseballbat1_position += g_baseballbat1_movement * g_baseballbat1_speed * delta_time;

    g_youwin_matrix = glm::mat4(1.0f);
    g_baseballbat1_matrix = glm::mat4(1.0f);
    g_baseballbat2_matrix = glm::mat4(1.0f);
    g_baseball_matrix = glm::mat4(1.0f);

    if (!baseballbat2_youwin && !baseballbat1_youwin) {
        g_youwin_matrix = glm::translate(g_youwin_matrix, glm::vec3(10.0f, 0.0f, 0.0f));
    } else if (baseballbat2_youwin) {
        g_youwin_matrix = glm::translate(g_youwin_matrix, glm::vec3(3.0f, 0.0f, 0.0f));
    } else if (baseballbat1_youwin) {
        g_youwin_matrix = glm::translate(g_youwin_matrix, glm::vec3(-3.0f, 0.0f, 0.0f));
    }

    g_youwin_matrix = glm::scale(g_youwin_matrix, glm::vec3(2.0f, 2.0f, 0.0f));

    g_baseballbat1_matrix = glm::translate(g_baseballbat1_matrix, g_baseballbat1_position);
    if (ai_mode) {
        g_baseballbat1_matrix = glm::mat4(1.0f);
        if (baseballbat1_collision_top_ai || baseballbat1_collision_bottom_ai) {
            increment = -increment;
        }
        direction += increment * delta_time;
        g_baseballbat1_matrix = glm::translate(g_baseballbat1_matrix, glm::vec3(0.0f, direction, 0.0f));

        float y_baseballbat1_distance_top_ai = (direction + INIT_POS_baseballbat1.y + INIT_baseballbat1_SCALE.y / 2.0f) - 3.75f;
        float y_baseballbat1_distance_bottom_ai = (direction + INIT_POS_baseballbat1.y - INIT_baseballbat1_SCALE.y / 2.0f) + 3.75f;

        if (y_baseballbat1_distance_top_ai > 0) {
            baseballbat1_collision_top_ai = true;
        } else {
            baseballbat1_collision_top_ai = false;
        }
        if (y_baseballbat1_distance_bottom_ai < 0) {
            baseballbat1_collision_bottom_ai = true;
        } else {
            baseballbat1_collision_bottom_ai = false;
        }
    }
    g_baseballbat1_matrix = glm::translate(g_baseballbat1_matrix, INIT_POS_baseballbat1);
    g_baseballbat1_matrix = glm::scale(g_baseballbat1_matrix, INIT_baseballbat1_SCALE);

    g_baseballbat2_matrix = glm::translate(g_baseballbat2_matrix, INIT_POS_baseballbat2);
    g_baseballbat2_matrix = glm::translate(g_baseballbat2_matrix, g_baseballbat2_position);
    g_baseballbat2_matrix = glm::scale(g_baseballbat2_matrix, INIT_baseballbat2_SCALE);


    if (g_baseballbat1_position.y + INIT_POS_baseballbat1.y + (INIT_baseballbat1_SCALE.y / 2.0f) > 3.75f) {
        g_baseballbat1_position.y = 3.75f - INIT_POS_baseballbat1.y - (INIT_baseballbat1_SCALE.y / 2.0f);
        baseballbat1_collision_top = true;
    } else if (g_baseballbat1_position.y + INIT_POS_baseballbat1.y - (INIT_baseballbat1_SCALE.y / 2.0f) < -3.75f) {
        g_baseballbat1_position.y = -3.75f - INIT_POS_baseballbat1.y + (INIT_baseballbat1_SCALE.y / 2.0f);
        baseballbat1_collision_bottom = true;
    } else {
        baseballbat1_collision_top = false;
        baseballbat1_collision_bottom = false;
    }
    // Right (baseball bat 2)
    if (g_baseballbat2_position.y + INIT_POS_baseballbat2.y + (INIT_baseballbat2_SCALE.y / 2.0f) > 3.75f) {
        g_baseballbat2_position.y = 3.75f - INIT_POS_baseballbat2.y - (INIT_baseballbat2_SCALE.y / 2.0f);
        baseballbat2_collision_top = true;
    } else if (g_baseballbat2_position.y + INIT_POS_baseballbat2.y - (INIT_baseballbat2_SCALE.y / 2.0f) < -3.75f) {
        g_baseballbat2_position.y = -3.75f - INIT_POS_baseballbat2.y + (INIT_baseballbat2_SCALE.y / 2.0f);
        baseballbat2_collision_bottom = true;
    } else {
        baseballbat2_collision_top = false;
        baseballbat2_collision_bottom = false;
    }

    // Baseball collision handling
    g_baseball_matrix = glm::mat4(1.0f);
    if (baseball_collision_top || baseball_collision_bottom) {
        baseball_increment_y = -baseball_increment_y;
    }
    if (baseball_collision_right || baseball_collision_left) {
        baseball_increment_x = 0;
        baseball_increment_y = 0;
    }
    if (game_start) {
        g_baseball_position.x += baseball_increment_x * delta_time;
        g_baseball_position.y += baseball_increment_y * delta_time;
    }

    // Top and bottom baseball collision
    float y_baseball_distance_top = (g_baseball_position.y + INIT_baseball_SCALE.y / 2.0f) - 3.75f;
    float y_baseball_distance_bottom = (g_baseball_position.y - INIT_baseball_SCALE.y / 2.0f) + 3.75f;

    // Left and right baseball collision
    float x_baseball_distance_right = (g_baseball_position.x + INIT_baseball_SCALE.x / 2.0f) - 5.0f;
    float x_baseball_distance_left = (g_baseball_position.x - INIT_baseball_SCALE.x / 2.0f) + 5.0f;

    if (y_baseball_distance_top > 0) {
        baseball_collision_top = true;
    } else {
        baseball_collision_top = false;
    }
    if (y_baseball_distance_bottom < 0) {
        baseball_collision_bottom = true;
    } else {
        baseball_collision_bottom = false;
    }

    if (x_baseball_distance_right > 0) {
        baseball_collision_right = true;
        baseballbat1_youwin = true;
    } else {
        baseball_collision_right = false;
    }
    if (x_baseball_distance_left < 0) {
        baseball_collision_left = true;
        baseballbat2_youwin = true;
    } else {
        baseball_collision_left = false;
    }

    float collision_factor = 0.5f;

    float x_baseball_distance_bat2 = fabs(g_baseball_position.x - INIT_POS_baseballbat2.x) -
        ((INIT_baseballbat2_SCALE.x * collision_factor + INIT_baseball_SCALE.x * collision_factor) / 2.0f);
    float y_baseball_distance_bat2 = fabs(g_baseball_position.y - (g_baseballbat2_position.y + INIT_POS_baseballbat2.y)) -
        ((INIT_baseballbat2_SCALE.y * collision_factor + INIT_baseball_SCALE.y * collision_factor) / 2.0f);

    float x_baseball_distance_bat1 = fabs(g_baseball_position.x - INIT_POS_baseballbat1.x) -
        ((INIT_baseballbat1_SCALE.x * collision_factor + INIT_baseball_SCALE.x * collision_factor) / 2.0f);
    float y_baseball_distance_bat1 = fabs(g_baseball_position.y - (g_baseballbat1_position.y + INIT_POS_baseballbat1.y)) -
        ((INIT_baseballbat1_SCALE.y * collision_factor + INIT_baseball_SCALE.y * collision_factor) / 2.0f);

    if (x_baseball_distance_bat2 <= 0.0f && y_baseball_distance_bat2 <= 0.0f) {
        baseball_increment_x = -baseball_increment_x;
    }
    if (x_baseball_distance_bat1 <= 0.0f && y_baseball_distance_bat1 <= 0.0f) {
        baseball_increment_x = -baseball_increment_x;
    }

    if (game_start) {
        g_baseball_matrix = glm::translate(g_baseball_matrix, g_baseball_position);
    }
}


void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); 
}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 background_matrix = glm::mat4(1.0f);
    background_matrix = glm::scale(background_matrix, glm::vec3(10.0f, 7.5f, 1.0f)); 

    g_shader_program.set_model_matrix(background_matrix);
    glBindTexture(GL_TEXTURE_2D, g_background_texture_id);

    // for bg
    float bg_vertices[] = {
        -0.5f, -0.5f, // bottom left
        0.5f, -0.5f,  // bottom right
        0.5f, 0.5f,   // top right
        -0.5f, -0.5f, // bottom left
        0.5f, 0.5f,   // top right
        -0.5f, 0.5f   // top left
    };

    float bg_texture_coords[] = {
        0.0f, 1.0f, // bottom left
        1.0f, 1.0f, // bottom right
        1.0f, 0.0f, // top right
        0.0f, 1.0f, // bottom left
        1.0f, 0.0f, // top right
        0.0f, 0.0f  // top left
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, bg_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, bg_texture_coords);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());


    //vertices
    float vertices[] =
    {
         -0.5f, -0.5f, 
         0.5f, -0.5f,  
         0.5f, 0.5f,
         -0.5f, -0.5f,
         0.5f, 0.5f,
         -0.5f, 0.5f
    };

    //textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 
        1.0f, 1.0f, 
        1.0f, 0.0f, 
        0.0f, 1.0f, 
        1.0f, 0.0f, 
        0.0f, 0.0f,  
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    //bind texture
    draw_object(g_baseballbat1_matrix, g_baseballbat1_texture_id);
    draw_object(g_baseballbat2_matrix, g_baseballbat2_texture_id);
    draw_object(g_baseball_matrix, g_baseball_texture_id);
    draw_object(g_youwin_matrix, g_youwin_texture_id);


    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
