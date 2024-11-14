#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Graphics/Device.hpp"


#include <iterator>

namespace EWE {
    PipelineBarrier::PipelineBarrier() :
        srcStageMask{},
        dstStageMask{},
        dependencyFlags{ 0 },
        memoryBarriers{},
        imageBarriers{},
        bufferBarriers{}
    {}
    PipelineBarrier::PipelineBarrier(PipelineBarrier& copySource) noexcept :
        srcStageMask{ copySource.srcStageMask },
        dstStageMask{ copySource.dstStageMask },
        dependencyFlags{ copySource.dependencyFlags },
        memoryBarriers{ std::move(copySource.memoryBarriers) },
        imageBarriers{ std::move(copySource.imageBarriers) },
        bufferBarriers{ std::move(copySource.bufferBarriers) }
    {}
    PipelineBarrier& PipelineBarrier::operator=(PipelineBarrier& copySource) {
        srcStageMask = copySource.srcStageMask;
        dstStageMask = copySource.dstStageMask;
        dependencyFlags = copySource.dependencyFlags;
        memoryBarriers = std::move(copySource.memoryBarriers);
        imageBarriers = std::move(copySource.imageBarriers);
        bufferBarriers = std::move(copySource.bufferBarriers);

        return *this;
    }
    PipelineBarrier::PipelineBarrier(PipelineBarrier&& moveSource) noexcept :
        srcStageMask{ moveSource.srcStageMask },
        dstStageMask{ moveSource.dstStageMask },
        dependencyFlags{ moveSource.dependencyFlags },
        memoryBarriers{ std::move(moveSource.memoryBarriers) },
        imageBarriers{ std::move(moveSource.imageBarriers) },
        bufferBarriers{ std::move(moveSource.bufferBarriers) }
    {}
    PipelineBarrier& PipelineBarrier::operator=(PipelineBarrier&& moveSource) {
        srcStageMask = moveSource.srcStageMask;
        dstStageMask = moveSource.dstStageMask;
        dependencyFlags = moveSource.dependencyFlags;
        memoryBarriers = std::move(moveSource.memoryBarriers);
        imageBarriers = std::move(moveSource.imageBarriers);
        bufferBarriers = std::move(moveSource.bufferBarriers);

        return *this;
    }


	void PipelineBarrier::Submit(VkCommandBuffer cmdBuf) const {
		EWE_VK(vkCmdPipelineBarrier, cmdBuf,
			srcStageMask, dstStageMask,
			dependencyFlags,
			memoryBarriers.size(), memoryBarriers.data(),
			bufferBarriers.size(), bufferBarriers.data(),
			imageBarriers.size(), imageBarriers.data()
		);
	}
	//the parameter object passed in is no longer usable, submitting both barriers will potentially lead to errors
	void PipelineBarrier::Merge(PipelineBarrier const& other) {
		//idk if i need if operators for empty vectors
		std::copy(other.memoryBarriers.begin(), other.memoryBarriers.end(), std::back_inserter(memoryBarriers));
		std::copy(other.bufferBarriers.begin(), other.bufferBarriers.end(), std::back_inserter(bufferBarriers));
		std::copy(other.imageBarriers.begin(), other.imageBarriers.end(), std::back_inserter(imageBarriers));
	}

	void PipelineBarrier::SimplifyVector(std::vector<PipelineBarrier>& barriers) {
		if (barriers.size() <= 1) {
			return;
		}
#if EWE_DEBUG
		assert(barriers.size() < 256 && "too many barriers"); //reduce the barrier count. if not possible, change the values and data types here
#endif

		uint8_t currentComparisonIndex = 1;
		int16_t nextComparisonIndex = -1;

		//c short for comparison
        while (currentComparisonIndex < barriers.size()) {
            nextComparisonIndex = barriers.size();

            const VkPipelineStageFlagBits cSrcStageMask = barriers[currentComparisonIndex].srcStageMask;
            const VkPipelineStageFlagBits cDstStageMask = barriers[currentComparisonIndex].dstStageMask;
            const VkDependencyFlags cDependencyFlags = barriers[currentComparisonIndex].dependencyFlags;

            for ( ; currentComparisonIndex < barriers.size(); currentComparisonIndex++) {
                const bool srcComp{ cSrcStageMask == barriers[currentComparisonIndex].srcStageMask };
                const bool dstComp{ cDstStageMask == barriers[currentComparisonIndex].dstStageMask };
                const bool dfComp{ cDependencyFlags == barriers[currentComparisonIndex].dependencyFlags };
                if (srcComp && dstComp && dfComp) {
                    barriers[currentComparisonIndex].Merge(barriers[currentComparisonIndex]);
                    barriers.erase(barriers.begin() + currentComparisonIndex);
                    currentComparisonIndex--;
                }
                else if (nextComparisonIndex > currentComparisonIndex) {
                    nextComparisonIndex = currentComparisonIndex;
                }
            }
            currentComparisonIndex = nextComparisonIndex;
        }
	}

	namespace Barrier {
		VkImageMemoryBarrier ChangeImageLayout(
			const VkImage image,
			const VkImageLayout oldImageLayout,
			const VkImageLayout newImageLayout,
			VkImageSubresourceRange const& subresourceRange
		) {
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			if ((oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
				imageMemoryBarrier.srcAccessMask = 0;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			}
			else {
				assert(false && "unsupported layout transition");
			}

			return imageMemoryBarrier;
		}

        VkImageMemoryBarrier TransitionImageLayout(VkImage& image, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t mipLevels, uint8_t layerCount) {

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext = nullptr;
            barrier.oldLayout = srcLayout;
            barrier.newLayout = dstLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = mipLevels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = layerCount;

            switch (srcLayout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter).
                // Only valid as initial layout. No flags required.
                barrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized.
                // Only valid as initial layout for linear images; preserves memory
                // contents. Make sure host writes have finished.
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment.
                // Make sure writes to the color buffer have finished
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment.
                // Make sure any writes to the depth/stencil buffer have finished.
                barrier.srcAccessMask
                    = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source.
                // Make sure any reads from the image have finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination.
                // Make sure any writes to the image have finished.
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader.
                // Make sure any shader reads from the image have finished
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier.srcAccessMask |= VK_ACCESS_SHADER_READ_BIT * (dstLayout == VK_IMAGE_LAYOUT_GENERAL);
                break;
            default:
                /* Value not used by callers, so not supported. */
                assert(false && "unsupported src layout transition");
            }

            // Target layouts (new)
            // The destination access mask controls the dependency for the new image
            // layout.
            switch (dstLayout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination.
                // Make sure any writes to the image have finished.
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source.
                // Make sure any reads from and writes to the image have finished.
                barrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment.
                // Make sure any writes to the color buffer have finished.
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment.
                // Make sure any writes to depth/stencil buffer have finished.
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment).
                // Make sure any writes to the image have finished.
                if (barrier.srcAccessMask == 0) {
                    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                /* Value not used by callers, so not supported. */
#if EWE_DEBUG
                assert(false && "unsupported dst layout transition");
#else
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
                __assume(false);
#else // GCC, Clang
                __builtin_unreachable();
#endif
#endif
            }

            return barrier;
        }
	
        void TransitionImageLayoutWithBarrier(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImage& image, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t mipLevels, uint8_t layerCount) {
            VkImageMemoryBarrier imageBarrier{ TransitionImageLayout(image, srcLayout, dstLayout, mipLevels, layerCount) };
            EWE_VK(vkCmdPipelineBarrier, cmdBuf,
                srcStageMask, dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageBarrier
            );
        }
}//namespace Barrier


    namespace Image {
        void GenerateMipMapsForMultipleImages(VkCommandBuffer cmdBuf, std::vector<MipParamPack>& mipParamPack) {
            //printf("before mip map loop? size of image : %d \n", image.size());

            //printf("after beginning single time command \n");
            uint8_t maxMipLevels;
            for (auto const& paramPack : mipParamPack) {
                if (paramPack.mipLevels > maxMipLevels) {
                    maxMipLevels = paramPack.mipLevels;
#if EWE_DEBUG
                    assert(paramPack.mipLevels >= 1);
#endif
                }
            }


            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext = nullptr;
            barrier.srcQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetTransferIndex();
            barrier.dstQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetGraphicsIndex(); //graphics queue is the only queue that can support vkCmdBlitImage
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            {
                const uint32_t mipLevel = 1;
                { //prebarrier, transfers ownership, prepares the current miplevel to be a transfer src
                    std::vector<VkImageMemoryBarrier> preBarriers{};
                    preBarriers.reserve(mipParamPack.size());
                    for (auto const& paramPack : mipParamPack) {

                        barrier.image = paramPack.image;
                        preBarriers.push_back(barrier);
                    }

                    EWE_VK(vkCmdPipelineBarrier, cmdBuf,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        preBarriers.size(), preBarriers.data()
                    );
                }
                { //blit, copies the current image to the next with linear scaling
                    VkImageBlit blit{};
                    blit.srcOffsets[0] = { 0, 0, 0 };
                    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.srcSubresource.mipLevel = mipLevel - 1;
                    blit.srcSubresource.baseArrayLayer = 0;
                    blit.srcSubresource.layerCount = 1;
                    blit.dstOffsets[0] = { 0, 0, 0 };
                    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.dstSubresource.mipLevel = mipLevel;
                    blit.dstSubresource.baseArrayLayer = 0;
                    blit.dstSubresource.layerCount = 1;
                    for (auto const& paramPack : mipParamPack) {
                        blit.srcOffsets[1] = VkOffset3D{ static_cast<int32_t>(paramPack.width), static_cast<int32_t>(paramPack.height), 1 };
                        blit.dstOffsets[1] = { paramPack.width > 1 ? static_cast<int32_t>(paramPack.width) / 2 : 1, paramPack.height > 1 ? static_cast<int32_t>(paramPack.height) / 2 : 1, 1 };
                        //printf("before blit image \n");
                        EWE_VK(vkCmdBlitImage, cmdBuf,
                            paramPack.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            paramPack.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1, &blit,
                            VK_FILTER_LINEAR
                        );
                    }
                }
                //printf("after blit image \n");
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                { //post barrier, finalizes the current mip level and prepares it for reading in the fragment stage
                    std::vector<VkImageMemoryBarrier> postBarriers{};
                    postBarriers.reserve(mipParamPack.size());
                    for (auto& paramPack : mipParamPack) {
                        barrier.image = paramPack.image;
                        postBarriers.push_back(barrier);
                        if (paramPack.width > 1) { paramPack.width /= 2; }
                        if (paramPack.height > 1) { paramPack.height /= 2; }
                    }
                    EWE_VK(vkCmdPipelineBarrier, cmdBuf,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        postBarriers.size(), postBarriers.data()
                    );
                }
                //printf("after pipeline barrier 2 \n");
            }

            for (uint32_t mipLevel = 2; mipLevel < maxMipLevels; mipLevel++) {
                barrier.subresourceRange.baseMipLevel = mipLevel - 1;
                std::vector<VkImageMemoryBarrier> preBarriers{};
                std::vector<VkImageMemoryBarrier> postBarriers{};

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;


                { //prebarrier, transfers ownership, prepares the current miplevel to be a transfer src
                    std::vector<VkImageMemoryBarrier> preBarriers{};
                    preBarriers.reserve(mipParamPack.size());
                    for (auto const& paramPack : mipParamPack) {

                        if (paramPack.mipLevels <= mipLevel) {
                            continue;
                        }
                        barrier.image = paramPack.image;
                        preBarriers.push_back(barrier);
                        //printf("before cmd pipeline barrier \n");
                        //this barrier right here needs a transfer queue partner
                    }

                    EWE_VK(vkCmdPipelineBarrier, cmdBuf,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        preBarriers.size(), preBarriers.data()
                    );
                }

                { //blit, copies the current image to the next with linear scaling
                    VkImageBlit blit{};
                    blit.srcOffsets[0] = { 0, 0, 0 };
                    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.srcSubresource.mipLevel = mipLevel - 1;
                    blit.srcSubresource.baseArrayLayer = 0;
                    blit.srcSubresource.layerCount = 1;
                    blit.dstOffsets[0] = { 0, 0, 0 };
                    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.dstSubresource.mipLevel = mipLevel;
                    blit.dstSubresource.baseArrayLayer = 0;
                    blit.dstSubresource.layerCount = 1;
                    for (auto const& paramPack : mipParamPack) {
                        if (paramPack.mipLevels <= mipLevel) {
                            continue;
                        }

                        blit.srcOffsets[1] = VkOffset3D{ static_cast<int32_t>(paramPack.width), static_cast<int32_t>(paramPack.height), 1 };
                        blit.dstOffsets[1] = { paramPack.width > 1 ? static_cast<int32_t>(paramPack.width) / 2 : 1, paramPack.height > 1 ? static_cast<int32_t>(paramPack.height) / 2 : 1, 1 };
                        //printf("before blit image \n");
                        EWE_VK(vkCmdBlitImage, cmdBuf,
                            paramPack.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            paramPack.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1, &blit,
                            VK_FILTER_LINEAR
                        );
                    }
                }
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                { //post barrier, finalizes the current mip level and prepares it for reading in the fragment stage
                    std::vector<VkImageMemoryBarrier> postBarriers{};
                    postBarriers.reserve(mipParamPack.size());
                    for (auto& paramPack : mipParamPack) {

                        if (paramPack.mipLevels <= mipLevel) {
                            continue;
                        }
                        barrier.image = paramPack.image;
                        postBarriers.push_back(barrier);
                        if (paramPack.width > 1) { paramPack.width /= 2; }
                        if (paramPack.height > 1) { paramPack.height /= 2; }
                    }
                    EWE_VK(vkCmdPipelineBarrier, cmdBuf,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        postBarriers.size(), postBarriers.data()
                    );
                }
            }

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            { //transition of the final mip level, which was copied to but never copied from, to be readable by the fragment shader
                std::vector<VkImageMemoryBarrier> finalBarriers{ mipParamPack.size() };
                for (uint8_t i = 0; i < mipParamPack.size(); i++) {
                    barrier.subresourceRange.baseMipLevel = mipParamPack[i].mipLevels - 1;
                    finalBarriers[i] = barrier;
                }

                EWE_VK(vkCmdPipelineBarrier, cmdBuf,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    finalBarriers.size(), finalBarriers.data()
                );
            }
        }

        void GenerateMipmaps(VkCommandBuffer cmdBuf, MipParamPack paramPack) {
            GenerateMipmaps(cmdBuf, paramPack.image, paramPack.mipLevels, paramPack.width, paramPack.height);
        }

        void GenerateMipmaps(VkCommandBuffer cmdBuf, VkImage image, uint8_t mipLevels, uint32_t width, uint32_t height) {
            //printf("before mip map loop? size of image : %d \n", image.size());

            //printf("after beginning single time command \n");

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext = nullptr;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetTransferIndex();
            barrier.dstQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetGraphicsIndex(); //graphics queue is the only queue that can support vkCmdBlitImage
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            for (uint32_t i = 1; i < mipLevels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                //printf("before cmd pipeline barrier \n");
                //this barrier right here needs a transfer queue partner
                EWE_VK(vkCmdPipelineBarrier, cmdBuf,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
                //printf("after cmd pipeline barreir \n");
                blit.srcOffsets[1] = VkOffset3D{ static_cast<int32_t>(width), static_cast<int32_t>(height), 1 };
                blit.srcSubresource.mipLevel = i - 1;
                blit.dstOffsets[1] = { width > 1 ? static_cast<int32_t>(width) / 2 : 1, height > 1 ? static_cast<int32_t>(height) / 2 : 1, 1 };
                blit.dstSubresource.mipLevel = i;
                //printf("before blit image \n");
                EWE_VK(vkCmdBlitImage, cmdBuf,
                    image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
                    VK_FILTER_LINEAR
                );
                //printf("after blit image \n");
                //this is going to be set again for each mip level, extremely small performance hit, potentially optimized away
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                EWE_VK(vkCmdPipelineBarrier, cmdBuf,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
                //printf("after pipeline barrier 2 \n");
                if (width > 1) { width /= 2; }
                if (height > 1) { height /= 2; }
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            //printf("before pipeline barrier 3 \n");
            EWE_VK(vkCmdPipelineBarrier,
                cmdBuf,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }
    }//namespace Image
} //namespace EWE