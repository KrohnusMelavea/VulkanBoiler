#include "EnumStrings.hpp"

#pragma warning(push, 0)
#include <ranges>
#pragma warning(pop)

#ifdef VK_ENABLE_BETA_EXTENSIONS
std::array<std::pair<VkBufferUsageFlagBits, std::string_view>, 27> inline constexpr VK_BUFFER_USAGE_FLAGS{ {
#else
std::array<std::pair<VkBufferUsageFlagBits, std::string_view>, 24> static constexpr VK_BUFFER_USAGE_FLAGS{ {
#endif
    { VK_BUFFER_USAGE_TRANSFER_SRC_BIT, "VK_BUFFER_USAGE_TRANSFER_SRC_BIT" },
    { VK_BUFFER_USAGE_TRANSFER_DST_BIT, "VK_BUFFER_USAGE_TRANSFER_DST_BIT" },
    { VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT" },
    { VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT" },
    { VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, "VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT" },
    { VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "VK_BUFFER_USAGE_STORAGE_BUFFER_BIT" },
    { VK_BUFFER_USAGE_INDEX_BUFFER_BIT, "VK_BUFFER_USAGE_INDEX_BUFFER_BIT" },
    { VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT" },
    { VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT" },
    { VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT / VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT / VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR" },
    { VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR, "VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR" },
    { VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR, "VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR" },
    { VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT, "VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT" },
    { VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT, "VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT" },
    { VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT, "VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT" },
#ifdef VK_ENABLE_BETA_EXTENSIONS
    { VK_BUFFER_USAGE_EXECUTION_GRAPH_SCRATCH_BIT_AMDX, "VK_BUFFER_USAGE_EXECUTION_GRAPH_SCRATCH_BIT_AMDX" },
#endif
    { VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, "VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR" },
    { VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, "VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR" },
    { VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR, "VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR / VK_BUFFER_USAGE_RAY_TRACING_BIT_NV" },
#ifdef VK_ENABLE_BETA_EXTENSIONS
    { VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR, "" },
    { VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR, "" },
#endif
    { VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, "VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT" },
    { VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, "VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT" },
    { VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT, "VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT" },
    { VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT, "VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT" },
    { VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT, "VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT" },
    { VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM, "VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM" }
} };
std::array<std::pair<VkMemoryPropertyFlagBits, std::string_view>, 10> static constexpr VK_MEMORY_PROPERTY_FLAGS{ {
    { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" },
    { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" },
    { VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" },
    { VK_MEMORY_PROPERTY_HOST_CACHED_BIT, "VK_MEMORY_PROPERTY_HOST_CACHED_BIT" },
    { VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" },
    { VK_MEMORY_PROPERTY_PROTECTED_BIT, "VK_MEMORY_PROPERTY_PROTECTED_BIT" },
    { VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD, "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD" },
    { VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD, "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD" },
    { VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV, "VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV" },
    { VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM, "VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM" }
} };

std::string getEnumString(VkFlags const vk_flags, FlagType const flag_type) {
    std::string flags;
    switch (flag_type) {
    case FlagType::BufferUsage:
        std::ranges::copy(VK_BUFFER_USAGE_FLAGS | std::views::filter([vk_flags](auto&& flag) { return (vk_flags & flag.first) != 0; }) | std::views::transform([](auto&& flag) { return std::string(flag.second); }) | std::views::join_with(std::string(" & ")), std::back_inserter(flags));
        break;
    case FlagType::MemoryProperty:
        std::ranges::copy(VK_MEMORY_PROPERTY_FLAGS | std::views::filter([vk_flags](auto&& flag) { return (vk_flags & flag.first) != 0; }) | std::views::transform([](auto&& flag) { return std::string(flag.second); }) | std::views::join_with(std::string(" & ")), std::back_inserter(flags));
        break;
    }
    
    return flags;
}