#include "EWEngine/Graphics/DebugNaming.h"

namespace EWE{
    namespace DebugNaming{
        PFN_vkDebugMarkerSetObjectTagEXT pfnDebugMarkerSetObjectTag{nullptr};
        PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName{};
        PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin{};
        PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd{};
        PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert{};
        bool enabled = false;

        void Initialize(VkDevice device, bool extension_enabled){
            if(extension_enabled){
                pfnDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
                pfnDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
                pfnCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
                pfnCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
                pfnCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
                enabled = 
                    (pfnDebugMarkerSetObjectTag != nullptr) &&
                    (pfnDebugMarkerSetObjectName != nullptr) &&
                    (pfnCmdDebugMarkerBegin != nullptr) &&
                    (pfnCmdDebugMarkerEnd != nullptr) &&
                    (pfnCmdDebugMarkerInsert != nullptr)
                ;
                assert(enabled && "failed to enable DEBUG_MARKER extension");
            }
        }
        void Deconstruct(){}
        void SetObjectName(VkDevice device, void* object, VkDebugReportObjectTypeEXT objectType, const char *name) {
                // Check for a valid function pointer
                if (enabled) {
                    VkDebugMarkerObjectNameInfoEXT nameInfo = {};
                    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
                    nameInfo.objectType = objectType;
                    nameInfo.object = reinterpret_cast<uint64_t>(object);
                    nameInfo.pObjectName = name;
                    pfnDebugMarkerSetObjectName(device, &nameInfo);
                }
            }
    } //namespace DebugNaming
} //namespace EWE