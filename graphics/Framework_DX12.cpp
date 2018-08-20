#include "stdafx.h"
#include "Framework_DX12.h"
#include "Win32Application.h"

namespace
{
    using DXGIFactoryInterface = IDXGIFactory6;

    // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
    // If no such adapter can be found, *ppAdapter will be set to nullptr.
    _Use_decl_annotations_
        void GetHardwareAdapter(DXGIFactoryInterface* pFactory, IDXGIAdapter1** ppAdapter, DXGI_ADAPTER_DESC1& hardwareAdapterDesc)
    {
        ComPtr<IDXGIAdapter1> adapter;
        *ppAdapter = nullptr;

        SIZE_T maxDedicatedVideoMemory = 0;

        ComPtr<IDXGIAdapter1> chosenAdapter;
        DXGI_ADAPTER_DESC1 chosenAdapterDesc{};

        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)) &&
                desc.DedicatedVideoMemory > maxDedicatedVideoMemory
                )
            {
                maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
                chosenAdapterDesc = desc;
                ThrowIfFailed(adapter.As(&chosenAdapter));
            }
        }

        *ppAdapter = chosenAdapter.Detach();
        hardwareAdapterDesc = chosenAdapterDesc;
    }

    const UINT PCI_ID_SOFTWARE_RASTERIZER = UINT(0x0001); //!< Software rasterizer.
    const UINT PCI_ID_AMD                 = UINT(0x1002); //!< AMD adapter.
    const UINT PCI_ID_INTEL               = UINT(0x8086); //!< Intel adapter.
    const UINT PCI_ID_NVIDIA              = UINT(0x10de); //!< nVidia adapter.

    const char* GetVendorName(UINT vendorId)
    {
        switch(vendorId)
        {
        case PCI_ID_SOFTWARE_RASTERIZER: return "software";
        case PCI_ID_AMD:    return "amd";
        case PCI_ID_INTEL:  return "intel";
        case PCI_ID_NVIDIA: return "nvidia";
        }
        return "unknown";
    }
}

Framework_DX12::Framework_DX12(UINT width, UINT height, bool useWarpDevice)
    : Framework(width, height)
    , m_useWarpDevice(useWarpDevice)
{
}

Framework_DX12::~Framework_DX12()
{
}

void Framework_DX12::Init()
{
    UINT dxgiFactoryFlags = 0;

    Win32Application::SetCustomWindowText(_T("Framework_DX12"));

    if (m_enableDebugLayer)
    {
        EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG; // Enable additional debug layers.
    }

    // Create DXGI Factory
    ComPtr<DXGIFactoryInterface> dxgiFactory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    DXGI_ADAPTER_DESC1 hardwareAdapterDesc;
    ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter(dxgiFactory.Get(), &hardwareAdapter, hardwareAdapterDesc);

    if (!hardwareAdapter.Get())
    {
    #if defined(CONF_BOOL_USE_WARP_DEVICE_IF_NO_HARDWARE_AVAILABLE)
        m_useWarpDevice = true;
    #else
        throw std::exception("No DirectX 12 compatible hardware found");
    #endif
    }

    ComPtr<DXGIAdapterInterface> dxgiAdapter;

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter1> dxgiAdapter1;
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        ThrowIfFailed(dxgiAdapter1.As(&dxgiFactory));
    }
    else
    {
        ThrowIfFailed(hardwareAdapter.As(&dxgiAdapter));
        auto vendorName = GetVendorName(hardwareAdapterDesc.VendorId);
        auto dedicatedVideoMemory = hardwareAdapterDesc.DedicatedVideoMemory;
        auto dedicatedSystemMemory = hardwareAdapterDesc.DedicatedSystemMemory;
        auto sharedSystemMemory = hardwareAdapterDesc.SharedSystemMemory;

        LOG("\tVendorName: %s"
            "\n\tDedicatedVideoMemory: %zu"
            "\n\tDedicatedSystemMemory: %zu"
            "\n\tSharedSystemMemory: %zu\n",
            vendorName, dedicatedVideoMemory, dedicatedSystemMemory, sharedSystemMemory
            );
    }

    m_device = CreateDevice(dxgiAdapter);

    m_commandQueue = CreateCommandQueue(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

    BOOL allowTearing = FALSE;
    if (FAILED(dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
    {
        LOG("Failed to check tearing support\n");
    }

    m_swapChain = CreateSwapChain(Win32Application::GetHwnd(), dxgiFactory, m_commandQueue, GetWidth(), GetHeight(), FrameBufferCount, allowTearing);
    
    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_rtvDescriptorHeap = CreateDescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FrameBufferCount);
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    UpdateRenderTargetViews(m_device, m_swapChain, m_rtvDescriptorHeap, FrameBufferCount);

    m_commandAllocator = CreateCommandAllocator(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_commandList = CreateCommandList(m_device, m_commandAllocator, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void Framework_DX12::Update()
{
    
}

void Framework_DX12::Render()
{
    
}

void Framework_DX12::Destroy()
{
    
}

void Framework_DX12::EnableDebugLayer() const
{
    ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
}

ComPtr<Framework_DX12::D3D12DeviceInterface> Framework_DX12::CreateDevice(ComPtr<DXGIAdapterInterface> adapter) const
{
    ComPtr<Framework_DX12::D3D12DeviceInterface> d3d12Device;
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device)));

    if (m_enableDebugLayer)
    {
        ComPtr<ID3D12InfoQueue> pInfoQueue;
        if (SUCCEEDED(d3d12Device.As(&pInfoQueue)))
        {
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            // Suppress whole categories of messages
            //D3D12_MESSAGE_CATEGORY Categories[] = {};

            // Suppress messages based on their severity level
            D3D12_MESSAGE_SEVERITY Severities[] =
            {
                D3D12_MESSAGE_SEVERITY_INFO
            };

            // Suppress individual messages by their ID
            D3D12_MESSAGE_ID DenyIds[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
            };

            D3D12_INFO_QUEUE_FILTER NewFilter = {};
            //NewFilter.DenyList.NumCategories = _countof(Categories);
            //NewFilter.DenyList.pCategoryList = Categories;
            NewFilter.DenyList.NumSeverities = _countof(Severities);
            NewFilter.DenyList.pSeverityList = Severities;
            NewFilter.DenyList.NumIDs = _countof(DenyIds);
            NewFilter.DenyList.pIDList = DenyIds;

            ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
        }
    }

    return d3d12Device;
}

ComPtr<Framework_DX12::D3D12CommandQueueInterface> Framework_DX12::CreateCommandQueue(ComPtr<D3D12DeviceInterface> device, D3D12_COMMAND_LIST_TYPE type) const
{
    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

    return d3d12CommandQueue;
}

ComPtr<Framework_DX12::DXGISwapChainInterface> Framework_DX12::CreateSwapChain(HWND hWnd, ComPtr<DXGIFactoryInterface> dxgiFactory, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount, bool supportTearing) const
{
    ComPtr<DXGISwapChainInterface> dxgiSwapChain;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = supportTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    ThrowIfFailed(dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    
    ThrowIfFailed(swapChain1.As(&dxgiSwapChain));

    return dxgiSwapChain;
}

ComPtr<Framework_DX12::D3D12DescriptorHeapInterface> Framework_DX12::CreateDescriptorHeap(ComPtr<D3D12DeviceInterface> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) const
{
    ComPtr<D3D12DescriptorHeapInterface> descriptorHeap;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0; // Zero for single-adapter operation

    //desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // can optionally be set on a descriptor heap to indicate it is be bound
    // to a command list for reference by shaders. Descriptor heaps created without this flag allow applications the option to stage 
    // descriptors in CPU memory before copying them to a shader visible descriptor heap, as a convenience.

    ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

void Framework_DX12::UpdateRenderTargetViews(ComPtr<D3D12DeviceInterface> device, ComPtr<DXGISwapChainInterface> swapChain, ComPtr<D3D12DescriptorHeapInterface> descriptorHeap, UINT nFrameBuffer)
{
    const auto RTVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < nFrameBuffer; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        m_backBuffers[i] = backBuffer;

        rtvHandle.Offset(RTVDescriptorSize);
    }
}

ComPtr<ID3D12CommandAllocator> Framework_DX12::CreateCommandAllocator(ComPtr<D3D12DeviceInterface> device, D3D12_COMMAND_LIST_TYPE type) const
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

    return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> Framework_DX12::CreateCommandList(ComPtr<D3D12DeviceInterface> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type) const
{
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    ThrowIfFailed(commandList->Close());

    return commandList;
}