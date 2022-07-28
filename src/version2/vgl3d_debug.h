#ifndef __VGL3D_DEBUG_H__
#define __VGL3D_DEBUG_H__

// ------------- Debug functions for vgl3d ----------------

#ifdef VITA

// as always rinne, thank you for this small code snippet
// Checks to make sure the uer has the shader compiler installed.
// If they don't, display a message and exit gracefully from the process.
static int userHasLibshaccg_private(VGL3DContext* context)
{
    SceCommonDialogConfigParam cmnDlgCfgParam;
    sceCommonDialogConfigParamInit(&cmnDlgCfgParam);

    SceIoStat st1, st2;
    if (!(sceIoGetstat("ur0:/data/libshacccg.suprx", &st1) >= 0 || sceIoGetstat("ur0:/data/external/libshacccg.suprx", &st2) >= 0)) {
        SceMsgDialogUserMessageParam msg_param;
        sceClibMemset(&msg_param, 0, sizeof(SceMsgDialogUserMessageParam));
        msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
        msg_param.msg = (const SceChar8*)"Error: Runtime shader compiler (libshacccg.suprx) is not installed.";
        context->Log(context, "\n\n\nError: Runtime shader compiler (libshacccg.suprx) is not installed.\n\n\n");
        SceMsgDialogParam param;
        sceMsgDialogParamInit(&param);
        param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
        param.userMsgParam = &msg_param;
        sceMsgDialogInit(&param);
        while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
            vglSwapBuffers(GL_TRUE);
        }
        sceKernelExitProcess(0);
    }

    return 1; // TRUE
}

#else

// Extern decls so we can access camEyePos and camEyeRot from this file.i
extern vec3 camEyePos_p;
extern vec3 camEyeRot_p;

void glfwError()
{
}

void __key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        const float amt = 30.0f;

        switch(scancode)
        {
            case GLFW_KEY_UP:
            case 126:
                camEyePos_p[1] += 0.5f;
                break;
            case GLFW_KEY_DOWN:
            case 125:
                camEyePos_p[1] -= 0.5f;
                break;
            case GLFW_KEY_RIGHT:
            case 124:
                camEyePos_p[0] += 0.5f;
                break;
            case GLFW_KEY_LEFT:
            case 123:
                camEyePos_p[0] -= 0.5f;
                break;
            case 33:
                camEyePos_p[2] -= 1.0f;
                break;
            case 30:
                camEyePos_p[2] += 1.0f;
                break;
            case 7:
                camEyeRot_p[0] += amt;
                break;
            case 6:
                camEyeRot_p[0] -= amt;
                break;
            case 0:
                camEyeRot_p[1] += amt;
                break;
            case 1:
                camEyeRot_p[1] -= amt;
                break;
            case 12:
                camEyeRot_p[2] += amt;
                break;
            case 13:
                camEyeRot_p[2] -= amt;
                break;
            case 35:
                VGL3D_Log(NULL, "(%.2f, %.2f, %.2f) Rot: (%.2f, %.2f, %.2f)\n", camEyePos_p[0], camEyePos_p[1], camEyePos_p[2], camEyeRot_p[0], camEyeRot_p[1], camEyeRot_p[2]);
                break;
            default:
                VGL3D_Log(NULL, "key: %d\n", scancode);
        }
    }
}
#endif


#define GLINVALIDENUM       "GL_INVALID_ENUM"
#define GLINVALIDVALUE      "GL_INVALID_VALUE"
#define GLINVALIDOP         "GL_INVALID_OPERATION"
#define GLOUTOFMEM          "GL_OUT_OF_MEMORY"
#define GLSTACKUNDER        "GL_STACK_UNDERFLOW"
#define GLSTACKOVER         "GL_STACK_OVERFLOW"
#define GLUNKNOWN           "GL_UNKNOWN. Sorry."

// This is honestly way better than my memcpy bullshit I was doing before.
#define SAFE_GET_GL_ERROR(x, y) \
switch(x) {\
    case (GL_INVALID_ENUM):\
        y = (GLINVALIDENUM);\
        break;\
    case (GL_INVALID_VALUE):\
        y = (GLINVALIDVALUE);\
        break;\
    case (GL_INVALID_OPERATION):\
        y = (GLINVALIDOP);\
        break;\
    case (GL_OUT_OF_MEMORY):\
        y = (GLOUTOFMEM);\
        break;\
    case (GL_STACK_UNDERFLOW):\
        y = (GLSTACKUNDER);\
        break;\
    case (GL_STACK_OVERFLOW):\
        y = (GLSTACKOVER);\
        break;\
    default:\
        y = (GLUNKNOWN);\
        break;\
}\

#ifndef SELF
#define SELF    VGL3DContext* context
#endif
static inline void CHECK_GL_ERROR(SELF, char* prefix) 
{
    GLenum gl_error = 0;
    const char *error_msg = NULL;
    if((gl_error = glGetError()) != GL_NO_ERROR)
    {
        SAFE_GET_GL_ERROR(gl_error, error_msg);
        context->Log(context, "[%s] OPENGL ERROR: %s\n", prefix, error_msg);
    }
}

#undef SELF
#undef GLINVALIDENUM  
#undef GLINVALIDVALUE 
#undef GLINVALIDOP    
#undef GLOUTOFMEM     
#undef GLSTACKUNDER   
#undef GLSTACKOVER    
#undef GLUNKNOWN      

// ------------- Debug functions for vgl3d ----------------

#endif //__VGL3D_DEBUG_H__