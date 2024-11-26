/* date = November 21st 2024 9:32 am */

#ifndef INPUT_H
#define INPUT_H

struct KeyData
{
    Int key_code;
    Bool just_pressed;
    Bool is_down;
    Float press_time;
    Float time_till_next_repeat;
};

struct Keys
{
    union
    {
        union
        {
            struct
            {
                KeyData left;
                KeyData right;
                KeyData up;
                KeyData down;
                KeyData page_up;
                KeyData page_down;
                
                KeyData a;
                KeyData b;
                KeyData c;
                KeyData d;
                KeyData e;
                KeyData f;
                KeyData g;
                KeyData h;
                KeyData i;
                KeyData j;
                KeyData k;
                KeyData l;
                KeyData m;
                KeyData n;
                KeyData o;
                KeyData p;
                KeyData q;
                KeyData r;
                KeyData s;
                KeyData t;
                KeyData u;
                KeyData v;
                KeyData w;
                KeyData x;
                KeyData y;
                KeyData z;
                
                KeyData number_0;
                KeyData number_1;
                KeyData number_2;
                KeyData number_3;
                KeyData number_4;
                KeyData number_5;
                KeyData number_6;
                KeyData number_7;
                KeyData number_8;
                KeyData number_9;
                
                KeyData grave;
                KeyData minus;
                KeyData equal;
                KeyData left_bracket;
                KeyData right_bracket;
                KeyData backslash;
                KeyData semicolon;
                KeyData quote;
                KeyData slash;
                KeyData comma;
                KeyData period;
                
                KeyData space;
                KeyData backspace;
                KeyData del;
                KeyData tab;
                KeyData enter;
                KeyData caps_lock;
                KeyData escape;
                
                KeyData shift_left;
                KeyData shift_right;
                KeyData control_left;
                KeyData control_right;
                KeyData alt_left;
                KeyData alt_right;
                KeyData function;
                
                KeyData f_1;
                KeyData f_2;
                KeyData f_3;
                KeyData f_4;
                KeyData f_5;
                KeyData f_6;
                KeyData f_7;
                KeyData f_8;
                KeyData f_9;
                KeyData f_10;
                KeyData f_11;
                KeyData f_12;
            };
            struct
            {
                KeyData nav[6];
                KeyData letters[26];
                KeyData numbers[10];
                KeyData symbols[11];
                KeyData special[7];
                KeyData modifiers[7];
                KeyData functions[12];
            };
        };
        KeyData data[6+26+10+11+7+7+12];
    };
    
};

struct Input
{
    F32 key_first_repeat_time;
    F32 key_repeat_speed;
    
    V2 mouse_pos;
    V2 d_mouse_pos;
    
    Keys keys;
};

Void
fill_key_data(Input *input)
{
    input->key_first_repeat_time = 0.4f;
    input->key_repeat_speed = 0.02f;
    
    Keys *keys = &input->keys;
    
    for(int i = 0; i < sizeof(input->keys.data) / sizeof(KeyData); i++)
    {
        keys->data[i] = {0};
    }
    keys->left.key_code = GLFW_KEY_LEFT;
    keys->right.key_code = GLFW_KEY_RIGHT;
    keys->up.key_code = GLFW_KEY_UP;
    keys->down.key_code = GLFW_KEY_DOWN;
    keys->page_up.key_code = GLFW_KEY_PAGE_UP;
    keys->page_down.key_code = GLFW_KEY_PAGE_DOWN;
    //keys->Key.key_code = GLFW_KEY_;
    keys->a.key_code = GLFW_KEY_A;
    keys->b.key_code = GLFW_KEY_B;
    keys->c.key_code = GLFW_KEY_C;
    keys->d.key_code = GLFW_KEY_D;
    keys->e.key_code = GLFW_KEY_E;
    keys->f.key_code = GLFW_KEY_F;
    keys->g.key_code = GLFW_KEY_G;
    keys->h.key_code = GLFW_KEY_H;
    keys->i.key_code = GLFW_KEY_I;
    keys->j.key_code = GLFW_KEY_J;
    keys->k.key_code = GLFW_KEY_K;
    keys->l.key_code = GLFW_KEY_L;
    keys->m.key_code = GLFW_KEY_M;
    keys->n.key_code = GLFW_KEY_N;
    keys->o.key_code = GLFW_KEY_O;
    keys->p.key_code = GLFW_KEY_P;
    keys->q.key_code = GLFW_KEY_Q;
    keys->r.key_code = GLFW_KEY_R;
    keys->s.key_code = GLFW_KEY_S;
    keys->t.key_code = GLFW_KEY_T;
    keys->u.key_code = GLFW_KEY_U;
    keys->v.key_code = GLFW_KEY_V;
    keys->w.key_code = GLFW_KEY_W;
    keys->x.key_code = GLFW_KEY_X;
    keys->y.key_code = GLFW_KEY_Y;
    keys->z.key_code = GLFW_KEY_Z;
    //keys->Key.key_code = GLFW_KEY_;
    keys->number_0.key_code = GLFW_KEY_0;
    keys->number_1.key_code = GLFW_KEY_1;
    keys->number_2.key_code = GLFW_KEY_2;
    keys->number_3.key_code = GLFW_KEY_3;
    keys->number_4.key_code = GLFW_KEY_4;
    keys->number_5.key_code = GLFW_KEY_5;
    keys->number_6.key_code = GLFW_KEY_6;
    keys->number_7.key_code = GLFW_KEY_7;
    keys->number_8.key_code = GLFW_KEY_8;
    keys->number_9.key_code = GLFW_KEY_9;
    //keys->Key.key_code = GLFW_KEY_;
    keys->grave.key_code = GLFW_KEY_GRAVE_ACCENT;
    keys->minus.key_code = GLFW_KEY_MINUS;
    keys->equal.key_code = GLFW_KEY_EQUAL;
    keys->left_bracket.key_code = GLFW_KEY_LEFT_BRACKET;
    keys->right_bracket.key_code = GLFW_KEY_RIGHT_BRACKET;
    keys->backslash.key_code = GLFW_KEY_BACKSLASH;
    keys->semicolon.key_code = GLFW_KEY_SEMICOLON;
    keys->quote.key_code = GLFW_KEY_APOSTROPHE;
    keys->slash.key_code = GLFW_KEY_SLASH;
    keys->comma.key_code = GLFW_KEY_COMMA;
    keys->period.key_code = GLFW_KEY_PERIOD;
    //keys->Key.key_code = GLFW_KEY_;
    keys->space.key_code = GLFW_KEY_SPACE;
    keys->backspace.key_code = GLFW_KEY_BACKSPACE;
    keys->del.key_code = GLFW_KEY_DELETE;
    keys->tab.key_code = GLFW_KEY_TAB;
    keys->enter.key_code = GLFW_KEY_ENTER;
    keys->caps_lock.key_code = GLFW_KEY_CAPS_LOCK;
    keys->escape.key_code = GLFW_KEY_ESCAPE;
    //keys->Key.key_code = GLFW_KEY_;
    keys->shift_left.key_code = GLFW_KEY_LEFT_SHIFT;
    keys->shift_right.key_code = GLFW_KEY_RIGHT_SHIFT;
    keys->control_left.key_code = GLFW_KEY_LEFT_CONTROL;
    keys->control_right.key_code = GLFW_KEY_RIGHT_CONTROL;
    keys->alt_left.key_code = GLFW_KEY_LEFT_ALT;
    keys->alt_right.key_code = GLFW_KEY_RIGHT_ALT;
    keys->function.key_code = GLFW_KEY_RIGHT_SUPER;
    //keys->Key.key_code = GLFW_KEY_;
    keys->f_1.key_code = GLFW_KEY_F1;
    keys->f_2.key_code = GLFW_KEY_F2;
    keys->f_3.key_code = GLFW_KEY_F3;
    keys->f_4.key_code = GLFW_KEY_F4;
    keys->f_5.key_code = GLFW_KEY_F5;
    keys->f_6.key_code = GLFW_KEY_F6;
    keys->f_7.key_code = GLFW_KEY_F7;
    keys->f_8.key_code = GLFW_KEY_F8;
    keys->f_9.key_code = GLFW_KEY_F9;
    keys->f_10.key_code = GLFW_KEY_F10;
    keys->f_11.key_code = GLFW_KEY_F11;
    keys->f_12.key_code = GLFW_KEY_F12;
}

#if 0
Int key_code;
Bool just_pressed;
Bool is_down;
Float press_time;
Float time_till_next_repeat;
#endif

Void
update_key_input(Input *input, GLFWwindow *window, Float d_time)
{
    Keys *keys = &input->keys;
    for(int i = 0; i < sizeof(keys->data) / sizeof(KeyData); i++)
    {
        KeyData *key = &(keys->data[i]);
        
        if(glfwGetKey(window, key->key_code) == GLFW_PRESS)
        {
            key->is_down = true;
            if(key->press_time == 0)
                key->just_pressed = true;
            else
                key->just_pressed = false;
            
            key->press_time += d_time;
        }
        else
        {
            key->is_down = false;
            key->just_pressed = false;
            key->press_time = 0;
            key->time_till_next_repeat = 0;
        }
        
        if(key->just_pressed || key->press_time >= input->key_first_repeat_time)
        {
            if(key->time_till_next_repeat <= 0)
            {
                key->time_till_next_repeat = input->key_repeat_speed;
            }
            else
            {
                key->time_till_next_repeat -= d_time;
            }
        }
    }
}


#endif //INPUT_H
