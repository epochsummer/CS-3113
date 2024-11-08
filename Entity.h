enum AnimationDirection { UP, RIGHT, LEFT, DOWN };

class Entity
{
private:
    //Animation
    int m_walking[4][2];

    int* m_animation_indices = nullptr;
    int m_animation_frames;
    int m_animation_index = 0;
    int m_animation_rows;
    int m_animation_columns;

    float m_animation_time = 0.0f;

    //Transformations
    glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

    float m_speed; //move 1 unit per second

    glm::mat4 m_model_matrix;

    //Texture
    GLuint m_texture_id;

    //Gravity
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    //COLLISIONS
    float m_width = 1.0f;
    float m_height = 1.0f;


public:
    //-----STATIC VARIABLES------//
    static constexpr int SECONDS_PER_FRAME = 4;

    //-----METHODS------//
    //constructors
    Entity();
    Entity(GLuint texture_id, float speed, int m_walking[4][2], float animation_time,
        int animation_frames, int animation_index, int animation_cols,
        int animation_rows);
    Entity(GLuint texture_id, float speed); //simple constructor

    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id,
        int index);

    bool const check_collision(Entity* other) const;

    void update(float delta_time);
    void update(float delta_time, Entity* coolidable_entities, int collidable_entity_count, bool is_moving_h, bool is_moving_v);

    void render(ShaderProgram* program);

    void normalise_movement() { m_movement = glm::normalize(m_movement); };

    void face_left() { m_animation_indices = m_walking[LEFT]; }
    void face_right() { m_animation_indices = m_walking[RIGHT]; }
    void face_up() { m_animation_indices = m_walking[UP]; }
    void face_down() { m_animation_indices = m_walking[DOWN]; }

    void move_left() { m_acceleration.x = -1.0f; face_left(); };
    void move_right() { m_acceleration.x = 1.0f;  face_right(); };
    void move_up() { m_acceleration.y = 1.0f;  face_up(); };
    void move_down() { m_acceleration.y = -1.0f; face_down(); };

    // ————— GETTERS ————— //
    glm::vec3 const get_position()   const { return m_position; };
    glm::vec3 const get_movement()   const { return m_movement; };
    glm::vec3 const get_scale()      const { return m_scale; };
    GLuint    const get_texture_id() const { return m_texture_id; };
    float     const get_speed()      const { return m_speed; };
    glm::vec3 const get_acceleration() const { return m_acceleration; };
    glm::vec3 const get_velocity() const { return m_velocity; };
    float const get_width()         const { return m_width; }
    float const get_height()        const { return m_height; }

    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_acceleration(glm::vec3 new_position) { m_acceleration = new_position; }

    void const set_height(float new_height) { m_height = new_height; }
    void const set_width(float new_width) { m_width = new_width; }
    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_animation_cols(int new_cols) { m_animation_columns = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(int new_time) { m_animation_time = new_time; }


    // Setter for m_walking
    void set_walking(int walking[4][2])
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                m_walking[i][j] = walking[i][j];
            }
        }
    }
};
