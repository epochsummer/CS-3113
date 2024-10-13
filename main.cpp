/**
* Author: Seha Kim
* Assignment: Simple 2D Scene
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
flot g_previous_ticks = 0.0f;

constexpr float ROT_INCREMENT = 1.0f;

constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL    = 0,
                TEXTURE_BORDER     = 0;
int g_frame_counter = 0;

constexpr char baseballbat1_SPRITE_FILEPATH[]    = "baseballbat1.jpg";
constexpr char baseballbat2_SPRITE_FILEPATH[]    = "baseballbat2.jpg",
constexpr char baseball_SPRITE_FILEPATH[]    = "baseball.jpg",
constexpr char youwin_SPRITE_FILEPATH[]    = "youwin.jpg",

// vectors for calvin hobbes and brawlball
constexpr glm::vec3 INIT_RED_SCALE = glm::vec3(1.0f, 3.0f, 0.0f), // red, aka left player, aka hobbes
INIT_BLUE_SCALE = glm::vec3(1.0f, 3.0f, 0.0f), // blue, aka right player, aka calvin
INIT_brawlball_SCALE = glm::vec3(1.0f, 1.0f, 0.0f); // brawlball

constexpr glm::vec3 INIT_POS_RED = glm::vec3(-4.0f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_POS_BLUE = glm::vec3(4.0f, 0.0f, 0.0f);

bool game_start = false;

bool baseballbat1_collision_top = false;
bool baseballbat1_collision_bottom = false;
bool baseballbat1_collision_top_ai = false;
bool baseballbat1_collision_bottom_ai = false;

bool baseballbat2_collision_top = false;
bool baseballbat2_collision_bottom = false;

bool baseballbat1_win = false;
bool baseballbat2_win = false;


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
float brawlball_increment_y = 2.5f;
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
g_win_matrix,
g_projection_matrix;

GLuint g_baseballbat1_texture_id;
GLuint g_baseballbat2_texture_id;
GLuint g_baseball_texture_id;
GLuint g_win_texture_id;
GLuint g_background_texture_id;

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
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
    g_brawlball_texture_id = load_texture(BASEBALL_SPRITE_FILEPATH);
    g_youwin_texture_id = load_texture(YOU_WIN_FILEPATH);
    g_background_texture_id = load_texture("background.png");



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
                g_blue_movement.y = 1.0f;
                break;

            case SDLK_DOWN:
                g_blue_movement.y = -1.0f;
                break;

            case SDLK_q:
                g_app_status = TERMINATED;
                break;

            case SDLK_w:
                g_red_movement.y = 1.0f;
                break;
            case SDLK_s:
                g_red_movement.y = -1.0f;
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

    // normalize speeds of paddles
    if (glm::length(g_baseballbat2_movement) > 1.0f)
    {
        g_baseballbat2_movement = glm::normalize(g_blue_movement);
    }
    if (glm::length(g_baseballbat1_movement) > 1.0f)
    {
        g_baseballbat1_movement = glm::normalize(g_baseballbat1_movement);
    }

}


void update()
{
    /* Delta time calculations */
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    /* Game logic */
    g_rotation_othani.y += ROT_INCREMENT * delta_time;
    g_rotation_base.y += -1 * ROT_INCREMENT * delta_time;
    
    /* Model matrix reset */
    g_othani_matrix    = glm::mat4(1.0f);
    g_base_matrix = glm::mat4(1.0f);
    
    /* Transformations */
    g_othani_matrix = glm::translate(g_othani_matrix, INIT_POS_othani);
    g_othani_matrix = glm::rotate(g_othani_matrix,
                                  g_rotation_othani.y,
                                  glm::vec3(0.0f, 1.0f, 0.0f));
    g_othani_matrix = glm::scale(g_othani_matrix, INIT_SCALE);
    
    
    float radius = 2.0f;
    float angle = ticks;
    glm::vec3 othani_position = glm::vec3(g_othani_matrix[3][0], g_othani_matrix[3][1], g_othani_matrix[3][2]);
    g_base_matrix = glm::translate(g_base_matrix, othani_position + glm::vec3(radius * cos(angle), radius * sin(angle), 0.0f));
    g_base_matrix = glm::rotate(g_base_matrix,
                                g_rotation_base.y,
                                glm::vec3(0.0f, 1.0f, 0.0f));
    g_base_matrix = glm::scale(g_base_matrix, INIT_SCALE);
}


void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
                          0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                          false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_othani_matrix, g_othani_texture_id);
    draw_object(g_base_matrix, g_base_texture_id);

    // We disable two attribute arrays now
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
