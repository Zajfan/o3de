/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <Atom/RHI/ConstantsData.h>
#include <Atom/RHI/DeviceImage.h>
#include <Atom/RHI/DeviceImageView.h>
#include <Atom/RHI/DeviceBuffer.h>
#include <Atom/RHI/DeviceBufferView.h>
#include <AzCore/Preprocessor/Enum.h>

namespace AZ::RHI
{
    class DeviceShaderResourceGroup;
    class DeviceShaderResourceGroupPool;

    AZ_ENUM_CLASS_WITH_UNDERLYING_TYPE(
        BindlessResourceType,
        uint32_t,
        m_Texture2D, // ReadTexture
        m_RWTexture2D, // ReadWriteTexture
        m_TextureCube, // ReadTextureCube
        m_ByteAddressBuffer, // ReadBuffer
        m_RWByteAddressBuffer, // ReadWriteBuffer
        Count);

    //! Shader resource group data is a light abstraction over a flat table of shader resources
    //! and shader constants. It utilizes basic reflection information from the shader resource group layout
    //! to construct the table in the correct format for the platform-specific compile phase. The user
    //! is expected to create instances of this class, fill data, and then push it to an SRG instance.
    //!
    //! The shader resource group (SRG) includes a set of built-in SRG constants in a single internally-managed
    //! constant buffer. This is separate from any custom constant buffers that some SRG layouts may include
    //! as shader resources. SRG constants can be conveniently accessed through a variety of SetConstant.
    //!
    //! This data structure holds strong references to the resource views bound onto it.
    //!
    //! NOTE [Performance Warning]: This data structure allocates memory. If compiling several SRG's in a batch,
    //! prefer to share the data between them (i.e. within a single job).
    //!
    //! NOTE [SRG Constants]: The ConstantsData class is used for efficiently setting/getting the constants values of the SRG.
    class DeviceShaderResourceGroupData
    {
    public:
        //! By default creates an empty data structure. Must be initialized before use.
        DeviceShaderResourceGroupData();
        ~DeviceShaderResourceGroupData();

        //! Creates shader resource group data from a layout.
        explicit DeviceShaderResourceGroupData(const ShaderResourceGroupLayout* shaderResourceGroupLayout);

        //! Creates shader resource group data from a pool (usable on any SRG with the same layout).
        explicit DeviceShaderResourceGroupData(const DeviceShaderResourceGroupPool& shaderResourceGroupPool);

        //! Creates shader resource group data from an SRG instance (usable on any SRG with the same layout).
        explicit DeviceShaderResourceGroupData(const DeviceShaderResourceGroup& shaderResourceGroup);

        AZ_DEFAULT_COPY_MOVE(DeviceShaderResourceGroupData);

        //! Resolves a shader input name to an index using reflection. For performance reasons, this resolve
        //! operation should be performed once at initialization time (or as infrequently as possible).
        //! Assignment of shader inputs is faster when done using the shader input index directly.
        ShaderInputBufferIndex   FindShaderInputBufferIndex(const Name& name) const;
        ShaderInputImageIndex    FindShaderInputImageIndex(const Name& name) const;
        ShaderInputSamplerIndex  FindShaderInputSamplerIndex(const Name& name) const;
        ShaderInputConstantIndex FindShaderInputConstantIndex(const Name& name) const;
            
        //! Sets one image view for the given shader input index.
        bool SetImageView(ShaderInputImageIndex inputIndex, const DeviceImageView* imageView, uint32_t arrayIndex);

        //! Sets an array of image view for the given shader input index.
        bool SetImageViewArray(ShaderInputImageIndex inputIndex, AZStd::span<const DeviceImageView* const> imageViews, uint32_t arrayIndex = 0);

        //! Sets an unbounded array of image view for the given shader input index.
        bool SetImageViewUnboundedArray(ShaderInputImageUnboundedArrayIndex inputIndex, AZStd::span<const DeviceImageView* const> imageViews);

        //! Sets one buffer view for the given shader input index.
        bool SetBufferView(ShaderInputBufferIndex inputIndex, const DeviceBufferView* bufferView, uint32_t arrayIndex = 0);

        //! Sets an array of image view for the given shader input index.
        bool SetBufferViewArray(ShaderInputBufferIndex inputIndex, AZStd::span<const DeviceBufferView* const> bufferViews, uint32_t arrayIndex = 0);

        //! Sets an unbounded array of buffer view for the given shader input index.
        bool SetBufferViewUnboundedArray(ShaderInputBufferUnboundedArrayIndex inputIndex, AZStd::span<const DeviceBufferView* const> bufferViews);

        //! Sets one sampler for the given shader input index, using the bindingIndex as the key.
        bool SetSampler(ShaderInputSamplerIndex inputIndex, const SamplerState& sampler, uint32_t arrayIndex = 0);

        //! Sets an array of samplers for the given shader input index.
        bool SetSamplerArray(ShaderInputSamplerIndex inputIndex, AZStd::span<const SamplerState> samplers, uint32_t arrayIndex = 0);

        //! Assigns constant data for the given constant shader input index.
        bool SetConstantRaw(ShaderInputConstantIndex inputIndex, const void* bytes, uint32_t byteCount);
        bool SetConstantRaw(ShaderInputConstantIndex inputIndex, const void* bytes, uint32_t byteOffset, uint32_t byteCount);

        //! Assigns a value of type T to the constant shader input.
        template <typename T>
        bool SetConstant(ShaderInputConstantIndex inputIndex, const T& value);

        //! Assigns a specified number of rows from a Matrix
        template <typename T>
        bool SetConstantMatrixRows(ShaderInputConstantIndex inputIndex, const T& value, uint32_t rowCount);

        //! Assigns a value of type T to the constant shader input, at an array offset.
        template <typename T>
        bool SetConstant(ShaderInputConstantIndex inputIndex, const T& value, uint32_t arrayIndex);

        //! Assigns an array of type T to the constant shader input.
        template <typename T>
        bool SetConstantArray(ShaderInputConstantIndex inputIndex, AZStd::span<const T> values);

        //! Assigns constant data as a whole.
        //!
        //! CAUTION!
        //! Different platforms might follow different packing rules for the internally-managed SRG constant buffer.
        //! To set manually a constant buffer as a whole please use Constant Buffers in AZSL,
        //! instead of SRG Constants, then use RHI Buffers with constant binding flag and set
        //! the buffer memory following pragma 4 packing rule.
        bool SetConstantData(const void* bytes, uint32_t byteCount);
        bool SetConstantData(const void* bytes, uint32_t byteOffset, uint32_t byteCount);

        //! Returns a single image view associated with the image shader input index and array offset.
        const ConstPtr<DeviceImageView>& GetImageView(ShaderInputImageIndex inputIndex, uint32_t arrayIndex) const;

        //! Returns a span of image views associated with the given image shader input index.
        AZStd::span<const ConstPtr<DeviceImageView>> GetImageViewArray(ShaderInputImageIndex inputIndex) const;

        //! Returns an unbounded span of image views associated with the given buffer shader input index.
        AZStd::span<const ConstPtr<DeviceImageView>> GetImageViewUnboundedArray(ShaderInputImageUnboundedArrayIndex inputIndex) const;

        //! Returns a single buffer view associated with the buffer shader input index and array offset.
        const ConstPtr<DeviceBufferView>& GetBufferView(ShaderInputBufferIndex inputIndex, uint32_t arrayIndex) const;

        //! Returns a span of buffer views associated with the given buffer shader input index.
        AZStd::span<const ConstPtr<DeviceBufferView>> GetBufferViewArray(ShaderInputBufferIndex inputIndex) const;

        //! Returns an unbounded span of buffer views associated with the given buffer shader input index.
        AZStd::span<const ConstPtr<DeviceBufferView>> GetBufferViewUnboundedArray(ShaderInputBufferUnboundedArrayIndex inputIndex) const;

        //! Returns a single sampler associated with the sampler shader input index and array offset.
        const SamplerState& GetSampler(ShaderInputSamplerIndex inputIndex, uint32_t arrayIndex) const;

        //! Returns a span of samplers associated with the sampler shader input index.
        AZStd::span<const SamplerState> GetSamplerArray(ShaderInputSamplerIndex inputIndex) const;

        //! Returns constant data for the given shader input index as a template type.
        //! The stride of T must match the size of the constant input region. The number
        //! of elements in the returned span is the number of evenly divisible elements.
        //! If the strides do not match, an empty span is returned.
        template <typename T>
        AZStd::span<const T> GetConstantArray(ShaderInputConstantIndex inputIndex) const;

        //! Returns the constant data as type 'T' returned by value. The size of the constant region
        //! must match the size of T exactly. Otherwise, an empty instance is returned.
        template <typename T>
        T GetConstant(ShaderInputConstantIndex inputIndex) const;

        //! Treats the constant input as an array of type T, returning the element by value at the
        //! specified array index. The size of the constant region must equally partition into an
        //! array of type T. Otherwise, an empty instance is returned.
        template <typename T>
        T GetConstant(ShaderInputConstantIndex inputIndex, uint32_t arrayIndex) const;

        //! Returns constant data for the given shader input index as a span of bytes.
        AZStd::span<const uint8_t> GetConstantRaw(ShaderInputConstantIndex inputIndex) const;

        //! Returns a {Buffer, Image, Sampler} shader resource group. Each resource type has its own separate group.
        //!  - The size of this group matches the size provided by ShaderResourceGroupLayout::GetGroupSizeFor{Buffer, Image, Sampler}.
        //!  - Use ShaderResourceGroupLayout::GetGroupInterval to retrieve a [min, max) interval into the span.
        AZStd::span<const ConstPtr<DeviceImageView>> GetImageGroup() const;
        AZStd::span<const ConstPtr<DeviceBufferView>> GetBufferGroup() const;
        AZStd::span<const SamplerState> GetSamplerGroup() const;

        //! Reset image and buffer views setup for this DeviceShaderResourceGroupData
        //! So it won't hold references for any RHI resources
        void ResetViews();

        //! Returns the opaque constant data populated by calls to SetConstant and SetConstantData.
        //!
        //! CAUTION!
        //! Different platforms might follow different packing rules for the internally-managed SRG constant buffer.
        AZStd::span<const uint8_t> GetConstantData() const;

        //! Returns the underlying ConstantsData struct
        const ConstantsData& GetConstantsData() const;

        //! Returns the shader resource layout for this group.
        const ShaderResourceGroupLayout* GetLayout() const;

        enum class ResourceType : uint32_t
        {
            ConstantData,
            DeviceBufferView,
            DeviceImageView,
            BufferViewUnboundedArray,
            ImageViewUnboundedArray,
            Sampler,
            Count
        };

        enum class ResourceTypeMask : uint32_t
        {
            None = 0,
            ConstantDataMask = AZ_BIT(static_cast<uint32_t>(ResourceType::ConstantData)),
            BufferViewMask = AZ_BIT(static_cast<uint32_t>(ResourceType::DeviceBufferView)),
            ImageViewMask = AZ_BIT(static_cast<uint32_t>(ResourceType::DeviceImageView)),
            BufferViewUnboundedArrayMask = AZ_BIT(static_cast<uint32_t>(ResourceType::BufferViewUnboundedArray)),
            ImageViewUnboundedArrayMask = AZ_BIT(static_cast<uint32_t>(ResourceType::ImageViewUnboundedArray)),
            SamplerMask = AZ_BIT(static_cast<uint32_t>(ResourceType::Sampler))
        };

        // Structure to hold all the bindless views and the BindlessResourceType related to it
        struct BindlessResourceViews
        {
            BindlessResourceType m_bindlessResourceType = AZ::RHI::BindlessResourceType::Count;
            AZStd::vector<ConstPtr<DeviceResourceView>> m_bindlessResources;
        };
            
        //! Reset the update mask
        void ResetUpdateMask();

        //! Enable compilation for a resourceType specified by resourceTypeMask
        void EnableResourceTypeCompilation(ResourceTypeMask resourceTypeMask);

        //! Returns the mask that is suppose to indicate which resource type was updated
        uint32_t GetUpdateMask() const;
            
        //! Update the indirect buffer view with the indices of all the image views which reside in the global gpu heap.
        //! Ideally higher level code can access bindless heap indices directly from the view and populate any indirect
        //! buffer directly. This API is present in case we want RHI to track bindless resources which may be needed for
        //! backends like the metal.
        void SetBindlessViews(
            ShaderInputBufferIndex indirectResourceBufferIndex,
            const RHI::DeviceBufferView* indirectResourceBuffer,
            AZStd::span<const DeviceImageView* const> imageViews,
            uint32_t* outIndices,
            AZStd::span<bool> isViewReadOnly,
            uint32_t arrayIndex = 0);
            
        //! Update the indirect buffer view with the indices of all the buffer views which reside in the global gpu heap.
        //! Ideally higher level code can access bindless heap indices directly from the view and populate any indirect
        //! buffer directly. This API is present in case we want RHI to track bindless resources which may be needed for
        //! backends like the metal.
        void SetBindlessViews(
            ShaderInputBufferIndex indirectResourceBufferIndex,
            const RHI::DeviceBufferView* indirectResourceBuffer,
            AZStd::span<const DeviceBufferView* const> bufferViews,
            uint32_t* outIndices,
            AZStd::span<bool> isViewReadOnly,
            uint32_t arrayIndex = 0);

        //! Get the size of the bindless view map
        const uint32_t GetBindlessViewsSize() const;
            
        //! Return all the bindless views referenced indirectly  via SetBindlessViews api
        const AZStd::unordered_map<AZStd::pair<ShaderInputBufferIndex, uint32_t>, BindlessResourceViews>& GetBindlessResourceViews() const;
            
    private:
        static const ConstPtr<DeviceImageView> s_nullImageView;
        static const ConstPtr<DeviceBufferView> s_nullBufferView;
        static const SamplerState s_nullSamplerState;

        bool ValidateSetImageView(ShaderInputImageIndex inputIndex, const DeviceImageView* imageView, uint32_t arrayIndex) const;
        bool ValidateSetBufferView(ShaderInputBufferIndex inputIndex, const DeviceBufferView* bufferView, uint32_t arrayIndex) const;

        template<typename TShaderInput, typename TShaderInputDescriptor>
        bool ValidateImageViewAccess(TShaderInput inputIndex, const DeviceImageView* imageView, uint32_t arrayIndex) const;
        template<typename TShaderInput, typename TShaderInputDescriptor>
        bool ValidateBufferViewAccess(TShaderInput inputIndex, const DeviceBufferView* bufferView, uint32_t arrayIndex) const;

        ConstPtr<ShaderResourceGroupLayout> m_shaderResourceGroupLayout;

        //! The backing data store of bound resources for the shader resource group.
        AZStd::vector<ConstPtr<DeviceImageView>> m_imageViews;
        AZStd::vector<ConstPtr<DeviceBufferView>> m_bufferViews;
        AZStd::vector<SamplerState> m_samplers;
        AZStd::vector<ConstPtr<DeviceImageView>> m_imageViewsUnboundedArray;
        AZStd::vector<ConstPtr<DeviceBufferView>> m_bufferViewsUnboundedArray;

        // The map below is used to manage ownership of buffer and image views that aren't bound directly to the shader, but implicitly
        // referenced through indirection constants. The key corresponds to the pair of (buffer input slot, index) where the indirection
        // constants reside (an array of indirection buffers is supported)            
        AZStd::unordered_map<AZStd::pair<ShaderInputBufferIndex, uint32_t>, BindlessResourceViews> m_bindlessResourceViews;

        //! The backing data store of constants for the shader resource group.
        ConstantsData m_constantsData;

        //! Mask used to check whether to compile a specific resource type. This mask is managed by RPI and copied over to the RHI every frame. 
        uint32_t m_updateMask = 0;
    };

    template <typename T>
    bool DeviceShaderResourceGroupData::SetConstant(ShaderInputConstantIndex inputIndex, const T& value)
    {
        EnableResourceTypeCompilation(ResourceTypeMask::ConstantDataMask);
        return m_constantsData.SetConstant(inputIndex, value);
    }

    template <typename T>
    bool DeviceShaderResourceGroupData::SetConstant(ShaderInputConstantIndex inputIndex, const T& value, uint32_t arrayIndex)
    {
        EnableResourceTypeCompilation(ResourceTypeMask::ConstantDataMask);
        return m_constantsData.SetConstant(inputIndex, value, arrayIndex);
    }

    template<typename T>
    bool DeviceShaderResourceGroupData::SetConstantMatrixRows(ShaderInputConstantIndex inputIndex, const T& value, uint32_t rowCount)
    {
        EnableResourceTypeCompilation(ResourceTypeMask::ConstantDataMask);
        return m_constantsData.SetConstantMatrixRows(inputIndex, value, rowCount);
    }

    template <typename T>
    bool DeviceShaderResourceGroupData::SetConstantArray(ShaderInputConstantIndex inputIndex, AZStd::span<const T> values)
    {
        if (!values.empty())
        {
            EnableResourceTypeCompilation(ResourceTypeMask::ConstantDataMask);
        }
        return m_constantsData.SetConstantArray(inputIndex, values);
    }

    template <typename T>
    AZStd::span<const T> DeviceShaderResourceGroupData::GetConstantArray(ShaderInputConstantIndex inputIndex) const
    {
        return m_constantsData.GetConstantArray<T>(inputIndex);
    }

    template <typename T>
    T DeviceShaderResourceGroupData::GetConstant(ShaderInputConstantIndex inputIndex) const
    {
        return m_constantsData.GetConstant<T>(inputIndex);
    }

    template <typename T>
    T DeviceShaderResourceGroupData::GetConstant(ShaderInputConstantIndex inputIndex, uint32_t arrayIndex) const
    {
        return m_constantsData.GetConstant<T>(inputIndex, arrayIndex);
    }

    template<typename TShaderInput, typename TShaderInputDescriptor>
    bool DeviceShaderResourceGroupData::ValidateImageViewAccess(TShaderInput inputIndex, const DeviceImageView* imageView, [[maybe_unused]] uint32_t arrayIndex) const
    {
        if (!Validation::IsEnabled())
        {
            return true;
        }

        const TShaderInputDescriptor& shaderInputImage = GetLayout()->GetShaderInput(inputIndex);

        if (!imageView)
        {
            AZ_Error("DeviceShaderResourceGroupData", false,
                "Image Array Input '%s[%d]' is null.",
                shaderInputImage.m_name.GetCStr(), arrayIndex);
            return false;
        }

        const ImageViewDescriptor& imageViewDescriptor = imageView->GetDescriptor();
        const DeviceImage& image = imageView->GetImage();
        const ImageDescriptor& imageDescriptor = image.GetDescriptor();
        const ImageFrameAttachment* frameAttachment = image.GetFrameAttachment();

        // The image must have the correct bind flags for the slot.
        const bool isValidAccess =
            (shaderInputImage.m_access == ShaderInputImageAccess::Read && CheckBitsAll(imageDescriptor.m_bindFlags, ImageBindFlags::ShaderRead)) ||
            (shaderInputImage.m_access == ShaderInputImageAccess::ReadWrite && CheckBitsAll(imageDescriptor.m_bindFlags, ImageBindFlags::ShaderReadWrite));

        if (!isValidAccess)
        {
            AZ_Error("DeviceShaderResourceGroupData", false,
                "Image Input '%s[%d]': Invalid 'Read / Write' access. Expected '%s'.",
                shaderInputImage.m_name.GetCStr(), arrayIndex, GetShaderInputAccessName(shaderInputImage.m_access));
            return false;
        }

        if (shaderInputImage.m_access == ShaderInputImageAccess::ReadWrite)
        {
            // An image view assigned to an input with read-write access must be an attachment on the frame scheduler.
            if (!frameAttachment)
            {
                AZ_Error("DeviceShaderResourceGroupData", false,
                    "Image Input '%s[%d]': DeviceImage is bound to a ReadWrite shader input, "
                    "but it is not an attachment on the frame scheduler. All GPU-writable resources "
                    "must be declared as attachments in order to provide hazard tracking.",
                    shaderInputImage.m_name.GetCStr(), arrayIndex, GetShaderInputAccessName(shaderInputImage.m_access));
                return false;
            }

            // NOTE: We aren't able to validate the scope attachment here, because shader resource groups aren't directly
            // associated with a scope. Instead, the CommandListValidator class will check that the access is correct at
            // command list submission time.
        }

        auto checkImageType = [&imageDescriptor, &shaderInputImage, arrayIndex](ImageDimension expected)
        {
            if (imageDescriptor.m_dimension != expected)
            {
                AZ_UNUSED(shaderInputImage);
                AZ_UNUSED(arrayIndex);
                AZ_Error("DeviceShaderResourceGroupData", false,
                    "Image Input '%s[%d]': The image is %dD but the shader expected %dD",
                    shaderInputImage.m_name.GetCStr(),
                    arrayIndex,
                    static_cast<int>(imageDescriptor.m_dimension),
                    static_cast<int>(expected));
                return false;
            }
            return true;
        };

        switch (shaderInputImage.m_type)
        {
        case ShaderInputImageType::Unknown:
            // Unable to validate.
            break;

        case ShaderInputImageType::Image1DArray:
        case ShaderInputImageType::Image1D:
            if (!checkImageType(ImageDimension::Image1D))
            {
                return false;
            }
            break;

        case ShaderInputImageType::SubpassInput:
            if (!checkImageType(ImageDimension::Image2D))
            {
                return false;
            }
            break;

        case ShaderInputImageType::Image2DArray:
        case ShaderInputImageType::Image2D:
            if (!checkImageType(ImageDimension::Image2D))
            {
                return false;
            }
            if (imageDescriptor.m_multisampleState.m_samples != 1)
            {
                AZ_Error("DeviceShaderResourceGroupData", false,
                    "Image Input '%s[%d]': The image has multisample count %u but the shader expected 1.",
                    shaderInputImage.m_name.GetCStr(),
                    arrayIndex,
                    imageDescriptor.m_multisampleState.m_samples);
                return false;
            }
            break;

        case ShaderInputImageType::Image2DMultisample:
        case ShaderInputImageType::Image2DMultisampleArray:
            if (!checkImageType(ImageDimension::Image2D))
            {
                return false;
            }
            if (imageDescriptor.m_multisampleState.m_samples <= 1)
            {
                AZ_Error("DeviceShaderResourceGroupData", false,
                    "Image Input '%s[%d]': The image has multisample count %u but the shader expected more than 1.",
                    shaderInputImage.m_name.GetCStr(),
                    arrayIndex,
                    imageDescriptor.m_multisampleState.m_samples);
                return false;
            }
            break;

        case ShaderInputImageType::Image3D:
            if (!checkImageType(ImageDimension::Image3D))
            {
                return false;
            }
            break;

        case ShaderInputImageType::ImageCube:
        case ShaderInputImageType::ImageCubeArray:
            if (!checkImageType(ImageDimension::Image2D))
            {
                return false;
            }
            if (imageViewDescriptor.m_isCubemap == 0)
            {
                AZ_Error("DeviceShaderResourceGroupData", false,
                    "Image Input '%s[%d]': The shader expected a cubemap.",
                    shaderInputImage.m_name.GetCStr(),
                    arrayIndex);
                return false;
            }
            break;

        default:
            AZ_Assert(false, "Image Input '%s[%d]': Invalid image type!", shaderInputImage.m_name.GetCStr(), arrayIndex);
            return false;
        }

        return true;
    }

    template<typename TShaderInput, typename TShaderInputDescriptor>
    bool DeviceShaderResourceGroupData::ValidateBufferViewAccess(TShaderInput inputIndex, const DeviceBufferView* bufferView, [[maybe_unused]] uint32_t arrayIndex) const
    {
        if (!Validation::IsEnabled())
        {
            return true;
        }

        const TShaderInputDescriptor& shaderInputBuffer = GetLayout()->GetShaderInput(inputIndex);
        const BufferViewDescriptor& bufferViewDescriptor = bufferView->GetDescriptor();
        const DeviceBuffer& buffer = bufferView->GetBuffer();
        const BufferDescriptor& bufferDescriptor = buffer.GetDescriptor();
        const BufferFrameAttachment* frameAttachment = buffer.GetFrameAttachment();

        const bool isValidAccess =
            (shaderInputBuffer.m_access == ShaderInputBufferAccess::Constant && CheckBitsAll(bufferDescriptor.m_bindFlags, BufferBindFlags::Constant)) ||
            (shaderInputBuffer.m_access == ShaderInputBufferAccess::Read && CheckBitsAll(bufferDescriptor.m_bindFlags, BufferBindFlags::ShaderRead)) ||
            (shaderInputBuffer.m_access == ShaderInputBufferAccess::Read && CheckBitsAll(bufferDescriptor.m_bindFlags, BufferBindFlags::RayTracingAccelerationStructure)) ||
            (shaderInputBuffer.m_access == ShaderInputBufferAccess::ReadWrite && CheckBitsAll(bufferDescriptor.m_bindFlags, BufferBindFlags::ShaderReadWrite));

        if (!isValidAccess)
        {
            AZ_Error("DeviceShaderResourceGroupData", false,
                "Buffer Input '%s[%d]': Invalid 'Constant / Read / Write' access. Expected '%s'",
                shaderInputBuffer.m_name.GetCStr(), arrayIndex, GetShaderInputAccessName(shaderInputBuffer.m_access));
            return false;
        }

        if (shaderInputBuffer.m_access == ShaderInputBufferAccess::ReadWrite && !bufferView->IgnoreFrameAttachmentValidation())
        {
            // A buffer view assigned to an input with read-write access must be an attachment on the frame scheduler.
            if (!frameAttachment)
            {
                AZ_Error("DeviceShaderResourceGroupData", false,
                    "Buffer Input '%s[%d]': DeviceBuffer is bound to a ReadWrite shader input, "
                    "but it is not an attachment on the frame scheduler. All GPU-writable resources "
                    "must be declared as attachments in order to provide hazard tracking.",
                    shaderInputBuffer.m_name.GetCStr(), arrayIndex, GetShaderInputAccessName(shaderInputBuffer.m_access));
                return false;
            }

            // NOTE: We aren't able to validate the scope attachment here, because shader resource groups aren't directly
            // associated with a scope. Instead, the CommandListValidator class will check that the access is correct at
            // command list submission time.
        }

        if (shaderInputBuffer.m_type == ShaderInputBufferType::Constant)
        {
            //For Constant type the stride (full constant buffer) must be larger or equal than the buffer view size (element size x element count).
            if (!(shaderInputBuffer.m_strideSize >= (bufferViewDescriptor.m_elementSize * bufferViewDescriptor.m_elementCount)))
            {
                AZ_Error("DeviceShaderResourceGroupData", false,
                    "Buffer Input '%s[%d]': stride size %d must be larger than or equal to the buffer view size %d",
                    shaderInputBuffer.m_name.GetCStr(), arrayIndex, shaderInputBuffer.m_strideSize,
                    (bufferViewDescriptor.m_elementSize * bufferViewDescriptor.m_elementCount));
                return false;
            }
        }
        else if (shaderInputBuffer.m_type != ShaderInputBufferType::Typed)
        {
            // For any other type the buffer view's element size should match the stride.
            if (shaderInputBuffer.m_strideSize != bufferViewDescriptor.m_elementSize)
            {
                AZ_Error("DeviceShaderResourceGroupData", false, "Buffer Input '%s[%d]': Does not match expected stride size %d",
                    shaderInputBuffer.m_name.GetCStr(), arrayIndex, bufferViewDescriptor.m_elementSize);
                return false;
            }
        }

        bool isValidType = true;
        switch (shaderInputBuffer.m_type)
        {
        case ShaderInputBufferType::Unknown:
            // Unable to validate.
            break;

        case ShaderInputBufferType::Constant:
            // Element format is not relevant for viewing a buffer as a constant buffer, any format would be valid.
            break;

        case ShaderInputBufferType::Structured:
            isValidType &= bufferViewDescriptor.m_elementFormat == Format::Unknown;
            break;

        case ShaderInputBufferType::Typed:
            isValidType &= bufferViewDescriptor.m_elementFormat != Format::Unknown;
            break;

        case ShaderInputBufferType::Raw:
            isValidType &= bufferViewDescriptor.m_elementFormat == Format::R32_UINT;
            break;

        case ShaderInputBufferType::AccelerationStructure:
            isValidType &= bufferViewDescriptor.m_elementFormat == Format::R32_UINT;
            break;

        default:
            AZ_Assert(false, "Buffer Input '%s[%d]': Invalid buffer type!", shaderInputBuffer.m_name.GetCStr(), arrayIndex);
            return false;
        }

        if (!isValidType)
        {
            AZ_Error("DeviceShaderResourceGroupData", false,
                "Buffer Input '%s[%d]': Does not match expected type '%s'",
                shaderInputBuffer.m_name.GetCStr(), arrayIndex, GetShaderInputTypeName(shaderInputBuffer.m_type));
            return false;
        }

        return true;
    }
}
