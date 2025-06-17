#include "EdubotLib.hpp"    // Include the Edubot robot library for hardware control functions
#include <iostream>         // Include input/output stream for console messages

// Robot constants for logic and hardware parameters
const double FORWARD_SPEED = 0.4;               // Speed at which the robot moves forward
const double CRITICAL_FRONT_DISTANCE = 0.15;    // Minimum safe distance in meters to obstacle in front
const double WALL_LOST_DISTANCE = 1.0;          // Distance indicating no wall at the robot's right (outer corner)
const double EXIT_DISTANCE = 1.5;               // Distance to determine if robot has exited the maze
const int LOOP_DELAY_MS = 50;                   // Delay in milliseconds between each control loop
const int ROTATE_LEFT_DEGREES = -90;            // Angle in degrees for left turns
const int ROTATE_RIGHT_DEGREES = 90;            // Angle in degrees for right turns
const int WAIT_AFTER_ROTATE_MS = 1000;          // Time to wait after a rotation (ms)
const int WAIT_AFTER_EXIT_MS = 2000;            // Time to wait after detecting the exit (ms)
const int OUTER_CORNER_WAIT_MS = 600;           // Extra wait time before turning at an outer corner (ms)
const int MOVE_AFTER_TURN_MS = 750;             // Wait time after moving forward from an outer corner (ms)

// Robot states for the Pledge maze-solving algorithm
enum RobotState { 
    MOVING_STRAIGHT,    // Going straight through the maze
    WALL_FOLLOWING      // Following a wall after hitting an obstacle
};

int main() {
    EdubotLib edubot;                   // Create a robot interface instance

    if (!edubot.connect()) {            // Attempt to connect to the robot
        std::cout << "Unable to connect to the robot." << std::endl;
        return -1;                      // Exit if connection fails
    }

    std::cout << "Connected to the robot! Starting the Pledge Algorithm..." << std::endl;
    edubot.sleepMilliseconds(WAIT_AFTER_ROTATE_MS * 2);   // Wait 2 seconds before starting

    RobotState currentState = MOVING_STRAIGHT;             // Start in MOVING_STRAIGHT state
    double pledgeCounter = 0.0;                            // Counter for net angle turned (degrees)

    while (edubot.isConnected()) {         // Main control loop, runs while robot is connected

        // Check if all directions are clear and robot is moving straight (exit condition)
        if (currentState == MOVING_STRAIGHT &&
            edubot.getSonar(3) > EXIT_DISTANCE &&   // Front sensor: clear
            edubot.getSonar(6) > EXIT_DISTANCE &&   // Right sensor: clear
            edubot.getSonar(0) > EXIT_DISTANCE) {   // Left sensor: clear

            std::cout << "\n*****************************************" << std::endl;
            std::cout << "EXIT OF THE MAZE DETECTED!" << std::endl;
            std::cout << "Stopping the robot in 2 seconds..." << std::endl;
            std::cout << "*****************************************\n" << std::endl;

            edubot.stop();                               // Stop robot motors
            edubot.sleepMilliseconds(WAIT_AFTER_EXIT_MS); // Wait for 2 seconds
            break;                                       // Exit the main loop
        }

        // State machine for robot navigation
        switch (currentState) {
            case MOVING_STRAIGHT: {
                std::cout << "[State: MOVING STRAIGHT]" << std::endl;
                // If obstacle detected in front, switch to wall following
                if (edubot.getSonar(3) < CRITICAL_FRONT_DISTANCE) {
                    std::cout << "Obstacle detected! Starting wall following (Pledge)..." << std::endl;
                    edubot.stop();                           // Stop motors before turning
                    pledgeCounter = ROTATE_LEFT_DEGREES;      // Initialize pledge counter with the left turn
                    edubot.rotate(ROTATE_LEFT_DEGREES);       // Turn left 90 degrees
                    edubot.sleepMilliseconds(WAIT_AFTER_ROTATE_MS); // Wait for rotation completion
                    currentState = WALL_FOLLOWING;            // Change state to wall following
                } else {
                    edubot.move(FORWARD_SPEED);               // Move forward if path is clear
                }
                break;
            }

            case WALL_FOLLOWING: {
                // If the pledge counter returns to zero and front is clear, switch back to straight movement
                if (pledgeCounter == 0.0 && edubot.getSonar(3) > WALL_LOST_DISTANCE) {
                    std::cout << "Pledge counter is zero and path is clear! Resuming straight movement." << std::endl;
                    edubot.stop();                            // Stop before changing state
                    currentState = MOVING_STRAIGHT;            // Resume going straight
                    break;
                }

                double front_sonar = edubot.getSonar(3);      // Read front distance sensor
                double right_sonar = edubot.getSonar(6);      // Read right distance sensor

                std::cout << "[State: WALL FOLLOWING] Pledge: " << pledgeCounter 
                          << ", Front: " << front_sonar << "m, Right: " << right_sonar << "m" << std::endl;

                // If right wall is lost (outer corner), turn right
                if (right_sonar > WALL_LOST_DISTANCE) {
                    std::cout << "Outer corner detected. Making a wide right turn..." << std::endl;
                    edubot.sleepMilliseconds(OUTER_CORNER_WAIT_MS);   // Brief wait before turning
                    edubot.stop();                                    // Stop before turning
                    edubot.rotate(ROTATE_RIGHT_DEGREES);              // Turn right 90 degrees
                    pledgeCounter += ROTATE_RIGHT_DEGREES;            // Increment pledge counter
                    edubot.sleepMilliseconds(WAIT_AFTER_ROTATE_MS);   // Wait for rotation
                    edubot.move(FORWARD_SPEED);                       // Move forward into corridor
                    edubot.sleepMilliseconds(MOVE_AFTER_TURN_MS);     // Move ahead a bit after turn
                }
                // If obstacle in front (inner corner), turn left
                else if (front_sonar < CRITICAL_FRONT_DISTANCE) {
                    std::cout << "Inner corner detected. Turning left..." << std::endl;
                    edubot.stop();                                    // Stop before turning
                    edubot.rotate(ROTATE_LEFT_DEGREES);               // Turn left 90 degrees
                    pledgeCounter += ROTATE_LEFT_DEGREES;             // Decrement pledge counter
                    edubot.sleepMilliseconds(WAIT_AFTER_ROTATE_MS);   // Wait for rotation
                }
                // Otherwise, keep following the wall
                else {
                    edubot.move(FORWARD_SPEED);                       // Continue moving forward
                }
                break;
            }
        }

        edubot.sleepMilliseconds(LOOP_DELAY_MS);   // Wait a short period before next iteration
    }

    std::cout << "Mission complete." << std::endl;
    edubot.disconnect();                           // Disconnect from the robot hardware

    return 0;                                      // End program
}
