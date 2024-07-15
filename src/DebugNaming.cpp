#include "EWEngine/Graphics/DebugNaming.h"

namespace EWE{
#if DEBUG_NAMING
    namespace DebugNaming{
        PFN_vkQueueBeginDebugUtilsLabelEXT pfnQueueBegin;
        PFN_vkQueueEndDebugUtilsLabelEXT pfnQueueEnd;
        PFN_vkSetDebugUtilsObjectNameEXT pfnSetObjectName;
        bool enabled = false;

        void Initialize(VkDevice device, bool extension_enabled){
            if(extension_enabled){

                pfnQueueBegin = reinterpret_cast<PFN_vkQueueBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkQueueBeginDebugUtilsLabelEXT"));
                pfnQueueEnd = reinterpret_cast<PFN_vkQueueEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkQueueEndDebugUtilsLabelEXT"));
                pfnSetObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
                assert(pfnQueueBegin);
                assert(pfnQueueEnd);
                assert(pfnSetObjectName);
                enabled = pfnQueueBegin && pfnQueueEnd && pfnSetObjectName && extension_enabled;
                    
                assert(enabled && "failed to enable DEBUG_MARKER extension");
                
            }
        }
        void QueueBegin(VkQueue queue, float red, float green, float blue, const char* name) {
            VkDebugUtilsLabelEXT utilLabel{};
            utilLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            utilLabel.pNext = nullptr;
            utilLabel.color[0] = red;
            utilLabel.color[1] = green;
            utilLabel.color[2] = blue;
            utilLabel.color[3] = 1.f;
            utilLabel.pLabelName = name;

            pfnQueueBegin(queue, &utilLabel);
        }
        void QueueEnd(VkQueue queue) {
            pfnQueueEnd(queue);
        }

        void Deconstruct(){}
        void SetObjectName(VkDevice device, void* object, VkObjectType objectType, const char* name) {
                // Check for a valid function pointer
                if (enabled) {
                    VkDebugUtilsObjectNameInfoEXT nameInfo{};
                    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                    nameInfo.pNext = nullptr;
                    nameInfo.objectHandle = reinterpret_cast<uint64_t>(object);
                    nameInfo.objectType = objectType;
                    nameInfo.pObjectName = name;
                    //pfnDebugMarkerSetObjectName(device, &nameInfo);
                    pfnSetObjectName(device, &nameInfo);

                }
            }
    } //namespace DebugNaming
#endif
} //namespace EWE