#ifndef INPUTSERVICE_H
#define INPUTSERVICE_H

#define NUM_KEYS 105
#define HOLD_MS 1

#include "Timer.h"

#include <unordered_map>

#include <GLFW/glfw3.h>
#include <vector>
#include <cstring>

class InputService {
private:
    GLFWwindow* window;

    enum KeyState{
        IDLE,
        PRESS,
        HOLD
    };

    struct Key{
        KeyState state;
        Timepoint whenPressed;
    };

    Timepoint lastUpdateTime;

    Key keys[NUM_KEYS];

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        InputService * inputService = (InputService*)glfwGetWindowUserPointer(window);
        Timepoint now = Timer::GetCurrentTime();
        if (inputService)
            inputService->HandleKey(key, action, now);
        
    }

    void HandleKey(int key, int action, const Timepoint &  now) {

        if(action == GLFW_RELEASE){
            keys[key].state = KeyState::IDLE;
            return;
        }
            
        if(action == GLFW_PRESS && keys[key].state == KeyState::IDLE) {
            keys[key].state = KeyState::PRESS;
            keys[key].whenPressed = now;
        }

        if (action == GLFW_REPEAT && keys[key].state == KeyState::PRESS) {

            double duration = Timer::GetDurationInMiliseconds(now - keys[key].whenPressed);

            if (duration >= HOLD_MS){
                keys[key].state =  KeyState::HOLD;
            }
            
        }

    }
public:
    InputService(GLFWwindow* window){
        this->window = window;
        glfwSetKeyCallback(window, KeyCallback);

        memset(keys, 0, NUM_KEYS);
    }

    void Update() {
        Timepoint now = Timer::GetCurrentTime();
        double pressDuration = Timer::GetDurationInSeconds(now - lastUpdateTime);

        for(uint32_t key = 0; key < NUM_KEYS; ++key){

            if( glfwGetKey(window, key) == GLFW_RELEASE)
                keys[key].state = KeyState::IDLE;
            
        }

        lastUpdateTime = now;
    }

    bool IsPressed(int key) const{
        return keys[key].state == KeyState::PRESS;
    }

    bool IsMouseButtonPressed(int key) const{
        return glfwGetMouseButton(window, key);
    }

    bool IsHold(int key) const {
        return keys[key].state == KeyState::HOLD;
    }

};


#endif