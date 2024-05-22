#include "splashkit.h"
#include <vector>
#include <cmath>
#include <stdexcept>
#include <iostream>

const float GRAVITY = 9.8f; // define gravity constant, you can manipulate the value of this constant to make the gravity less or more aggressive

// define a 2df vector struct for storing position, velocity, and acceleration
struct Vector2D
{
    float x;
    float y;
};

// function to add two vectors
Vector2D vector_add(const Vector2D &a, const Vector2D &b)
{
    return {a.x + b.x, a.y + b.y}; // return the sum of the two vectors
}

// function to multiply a vector * scalar
Vector2D vector_multiply(const Vector2D &v, float scalar)
{
    return {v.x * scalar, v.y * scalar}; // return the vector multiplied by the scalar
}

// define a struct to represent objects in the simulation
struct PhysicsObject
{
    Vector2D position;
    Vector2D velocity;
    Vector2D acceleration;
    float mass;
    float radius;
    float width, height;
    color object_color;
    std::string type;
};

// function to update the physics of an object over time
void update_physics(PhysicsObject &obj, float dt)
{
    obj.acceleration.y = GRAVITY;
    obj.velocity = vector_add(obj.velocity, vector_multiply(obj.acceleration, dt));
    obj.position = vector_add(obj.position, vector_multiply(obj.velocity, dt)); // update position

    // handle screen collisions based on object type
    if (obj.type == "circle")
    {
        // handle circle collisions with screen edges
        if (obj.position.y + obj.radius > screen_height())
        {
            obj.position.y = screen_height() - obj.radius;
            obj.velocity.y *= -0.5;
        }
        if (obj.position.y - obj.radius < 0)
        {
            obj.position.y = obj.radius;
            obj.velocity.y *= -0.5;
        }
        if (obj.position.x + obj.radius > screen_width())
        {
            obj.position.x = screen_width() - obj.radius;
            obj.velocity.x *= -0.5;
        }
        if (obj.position.x - obj.radius < 0)
        {
            obj.position.x = obj.radius;
            obj.velocity.x *= -0.5;
        }
    }
    else
    {
        // handle rectangle collisions with screen edges
        if (obj.position.y + obj.height > screen_height())
        {
            obj.position.y = screen_height() - obj.height;
            obj.velocity.y *= -0.5;
        }
        if (obj.position.y < 0)
        {
            obj.position.y = 0;
            obj.velocity.y *= -0.5;
        }
        if (obj.position.x + obj.width > screen_width())
        {
            obj.position.x = screen_width() - obj.width;
            obj.velocity.x *= -0.5;
        }
        if (obj.position.x < 0)
        {
            obj.position.x = 0;
            obj.velocity.x *= -0.5;
        }
    }
}

// function to apply a force to an object
void apply_force(PhysicsObject &obj, const Vector2D &force)
{
    Vector2D acceleration = {force.x / obj.mass, force.y / obj.mass}; // calculate acceleration
    obj.acceleration = vector_add(obj.acceleration, acceleration); // update acceleration
}

// function to draw a physics object on the screen
void draw_object(const PhysicsObject &obj)
{
    // draw different shapes based on object type
    if (obj.type == "circle")
    {
        fill_circle(obj.object_color, obj.position.x, obj.position.y, obj.radius);
    }
    else if (obj.type == "rectangle")
    {
        fill_rectangle(obj.object_color, obj.position.x, obj.position.y, obj.width, obj.height);
    }
    else if (obj.type == "square")
    {
        fill_rectangle(obj.object_color, obj.position.x, obj.position.y, obj.width, obj.width);
    }
    else if (obj.type == "pixel")
    {
        draw_pixel(obj.object_color, obj.position.x, obj.position.y);
    }
}

// function to check for collision between two circle objects
bool circle_collision(const PhysicsObject &obj1, const PhysicsObject &obj2)
{
    // calculate distance between object centers
    float dx = obj1.position.x - obj2.position.x;
    float dy = obj1.position.y - obj2.position.y;
    float distance = sqrt(dx * dx + dy * dy);
    // check if distance is less than sum of radiuses
    return distance < (obj1.radius + obj2.radius);
}

// function to check for collision between two rectangle objects
bool rectangle_collision(const PhysicsObject &obj1, const PhysicsObject &obj2)
{
    // check if rectangles overlap in x and y dimensions
    return obj1.position.x < obj2.position.x + obj2.width &&
           obj1.position.x + obj1.width > obj2.position.x &&
           obj1.position.y < obj2.position.y + obj2.height &&
           obj1.position.y + obj1.height > obj2.position.y;
}

// function to check for collision between a circle and a rectangle
bool circle_rectangle_collision(const PhysicsObject &circle, const PhysicsObject &rect)
{
    // calculate closest point on rectangle to circle center
    float closest_x = fmax(rect.position.x, fmin(circle.position.x, rect.position.x + rect.width));
    float closest_y = fmax(rect.position.y, fmin(circle.position.y, rect.position.y + rect.height));
    float dx = circle.position.x - closest_x;
    float dy = circle.position.y - closest_y;
    // check if distance squared is less than square of circle radius
    return (dx * dx + dy * dy) < (circle.radius * circle.radius);
}

// function to check for collision between two objects based on their types
bool check_collision(const PhysicsObject &obj1, const PhysicsObject &obj2)
{
    if (obj1.type == "circle" && obj2.type == "circle")
    {
        return circle_collision(obj1, obj2);
    }
    else if (obj1.type == "rectangle" && obj2.type == "rectangle")
    {
        return rectangle_collision(obj1, obj2);
    }
    else if (obj1.type == "circle" && obj2.type == "rectangle")
    {
        return circle_rectangle_collision(obj1, obj2);
    }
    else if (obj1.type == "rectangle" && obj2.type == "circle")
    {
        return circle_rectangle_collision(obj2, obj1);
    }
    else if (obj1.type == "square" && obj2.type == "square")
    {
        return rectangle_collision(obj1, obj2);
    }
    else if (obj1.type == "circle" && obj2.type == "square")
    {
        return circle_rectangle_collision(obj1, obj2);
    }
    else if (obj1.type == "square" && obj2.type == "circle")
    {
        return circle_rectangle_collision(obj2, obj1);
    }
    return false;
}

// handling collisions
void handle_collision(PhysicsObject &obj1, PhysicsObject &obj2)
{
    Vector2D temp_velocity = obj1.velocity;
    obj1.velocity = obj2.velocity;
    obj2.velocity = temp_velocity;
}

// the main function
int main()
{
    open_window("2D Physics Simulator", 800, 600);

    std::vector<PhysicsObject> objects;

    while (!window_close_requested("2D Physics Simulator"))
    {
        process_events();

        if (mouse_clicked(LEFT_BUTTON)) // if the left mouse button is clicked
        {
            try // handle potential exceptions during user input
            {
                float x, y;
                write_line("Enter X coordinate: ");
                x = convert_to_integer(read_line());
                write_line("Enter Y coordinate: ");
                y = convert_to_integer(read_line());

                std::string type;
                float mass;
                color obj_color;

                write_line("Enter object type (circle, rectangle, square, pixel): ");
                type = read_line();

                if (type == "circle")
                {
                    float radius;
                    write_line("Enter radius: ");
                    radius = convert_to_integer(read_line());
                    mass = 1.0f;
                    obj_color = COLOR_RED;
                    if (radius <= 0)
                    {
                        throw std::invalid_argument("Radius must be greater than 0.");
                    }
                    objects.push_back({{x, y}, {0, 0}, {0, 0}, mass, radius, 0, 0, obj_color, type});
                }
                else if (type == "rectangle")
                {
                    float width, height;
                    write_line("Enter width and height: ");
                    width = convert_to_integer(read_line());
                    height = convert_to_integer(read_line());
                    mass = 1.0f;
                    obj_color = COLOR_GREEN;
                    if (width <= 0 || height <= 0)
                    {
                        throw std::invalid_argument("Width and height must be greater than 0.");
                    }
                    objects.push_back({{x, y}, {0, 0}, {0, 0}, mass, 0, width, height, obj_color, type});
                }
                else if (type == "square")
                {
                    float side;
                    write_line("Enter side length: ");
                    side = convert_to_integer(read_line());
                    mass = 1.0f;
                    obj_color = COLOR_BLUE;
                    if (side <= 0)
                    {
                        throw std::invalid_argument("Side length must be greater than 0.");
                    }
                    objects.push_back({{x, y}, {0, 0}, {0, 0}, mass, 0, side, 0, obj_color, type});
                }
                else if (type == "pixel")
                {
                    mass = 1.0f;
                    obj_color = COLOR_YELLOW;
                    objects.push_back({{x, y}, {0, 0}, {0, 0}, mass, 0, 0, 0, obj_color, type});
                }

                if (!objects.empty())
                {
                    PhysicsObject &obj = objects.back();
                    float force_x, force_y;

                    write_line("Enter X direction force: ");
                    force_x = convert_to_integer(read_line());
                    write_line("Enter Y direction force: ");
                    force_y = convert_to_integer(read_line());

                    apply_force(obj, {force_x, force_y});
                }
            }
            catch (const std::invalid_argument &e)
            {
                write_line("Invalid input: ");
                write_line(e.what());
                write_line("\n");
            }
            catch (const std::exception &e)
            {
                write_line("An error occurred: ");
                write_line(e.what());
                write_line("\n");
            }
        }

        for (PhysicsObject &obj : objects) // iterate over all objects
        {
            update_physics(obj, 1.0 / 60.0);
        }

        for (size_t i = 0; i < objects.size(); ++i)
        {
            for (size_t j = i + 1; j < objects.size(); ++j)
            {
                if (check_collision(objects[i], objects[j]))
                {
                    handle_collision(objects[i], objects[j]);
                }
            }
        }

        clear_screen(COLOR_WHITE);

        for (const PhysicsObject &obj : objects)
        {
            draw_object(obj);
        }

        refresh_screen(60);
    }

    return 0;
}
