
#include <cmath>
#include <windows.h>

#include "win32_graphics.h"
#include "graphics_math.cpp"
#include "dx12_rasterizer.cpp"
#include "assets.cpp"
#include <glm/glm.hpp>

global global_state GlobalState;

LRESULT Win32WindowCallBack(
  HWND WindowHandle,
  UINT Message,
  WPARAM WParam,
  LPARAM LParam
)
{
    LRESULT Result = {};

    switch (Message)
    {
        case WM_DESTROY:
        case WM_CLOSE:
        {
            GlobalState.IsRunning = false;
        } break;

        default:
        {
            Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
        } break;
    }
    
    return Result;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nShowCmd)
{
    GlobalState.IsRunning = true;
    LARGE_INTEGER TimerFrequency = {};
    Assert(QueryPerformanceFrequency(&TimerFrequency));
    
    {
        WNDCLASSA WindowClass = {};
        WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        WindowClass.lpfnWndProc = Win32WindowCallBack;
        WindowClass.hInstance = hInstance;
        WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
        WindowClass.lpszClassName = "dxd";
        if (!RegisterClassA(&WindowClass))
        {
            InvalidCodePath;
        }

        GlobalState.WindowHandle = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "dxd",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            1280,
            720,
            NULL,
            NULL,
            hInstance,
            NULL);

        if (!GlobalState.WindowHandle)
        {
            InvalidCodePath;
        }
    }

    GlobalState.RasterizerType = RasterizerType_Dx12;
    GlobalState.Dx12Rasterizer = Dx12RasterizerCreate(GlobalState.WindowHandle, 1920, 1080);

    GlobalState.CubeModel = AssetCreateCube(&GlobalState.Dx12Rasterizer);

    GlobalState.SponzaModel = AssetLoadModel(&GlobalState.Dx12Rasterizer, "Sponza\\", "Sponza.gltf");
    
    texture CheckerBoardTexture = {};
  

    dx12_rasterizer* Rasterizer = &GlobalState.Dx12Rasterizer;
    ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

    ThrowIfFailed(CommandList->Close());
    Rasterizer->CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&CommandList);

    Rasterizer->FenceValue += 1;
    ThrowIfFailed(Rasterizer->CommandQueue->Signal(Rasterizer->Fence, Rasterizer->FenceValue));
    if (Rasterizer->Fence->GetCompletedValue() != Rasterizer->FenceValue)
    {
        HANDLE FenceEvent = {};
        ThrowIfFailed(Rasterizer->Fence->SetEventOnCompletion(Rasterizer->FenceValue, FenceEvent));
        WaitForSingleObject(FenceEvent, INFINITE);
    }

    ThrowIfFailed(Rasterizer->CommandAllocator->Reset());
    ThrowIfFailed(CommandList->Reset(Rasterizer->CommandAllocator, 0));

    Dx12ClearUploadArena(&Rasterizer->UploadArena);
    
    LARGE_INTEGER BeginTime = {};
    LARGE_INTEGER EndTime = {};
    Assert(QueryPerformanceCounter(&BeginTime));
    
    while (GlobalState.IsRunning)
    {
        Assert(QueryPerformanceCounter(&EndTime));
        float FrameTime = float(EndTime.QuadPart - BeginTime.QuadPart) / float(TimerFrequency.QuadPart);
        BeginTime = EndTime;

        {
            char Text[256];
            snprintf(Text, sizeof(Text), "FrameTime: %f\n", FrameTime);
            OutputDebugStringA(Text);
        }
        
        MSG Message = {};
        while (PeekMessageA(&Message, GlobalState.WindowHandle, 0, 0, PM_REMOVE))
        {
            switch (Message.message)
            {
                case WM_QUIT:
                {
                    GlobalState.IsRunning = false;
                } break;

                case WM_KEYUP:
                case WM_KEYDOWN:
                {
                    int VkCode = Message.wParam;
                    int IsDown = !((Message.lParam >> 31) & 0x1);

                    switch (VkCode)
                    {
                        case 'W':
                        {
                            GlobalState.WDown = IsDown;
                        } break;

                        case 'A':
                        {
                            GlobalState.ADown = IsDown;
                        } break;

                        case 'S':
                        {
                            GlobalState.SDown = IsDown;
                        } break;

                        case 'D':
                        {
                            GlobalState.DDown = IsDown;
                        } break;
                    } 
                } break;
                
                default:
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                } break;
            }
        }
        
        RECT ClientRect = {};
        Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
        int ClientWidth = ClientRect.right - ClientRect.left;
        int ClientHeight = ClientRect.bottom - ClientRect.top;
        float AspectRatio = float(ClientWidth) / float(ClientHeight);
        
        // NOTE: Обчислюємо положення камери
        m4 CameraTransform = IdentityM4();
        {
            Camera* camera = &GlobalState.camera;

            float CameraRotationSpeed = 100.0f;

            int MouseDown = false;
            glm::vec2 CurrMousePos = {};
            if (GetActiveWindow() == GlobalState.WindowHandle)
            {
                POINT Win32MousePos = {}; 
                Assert(GetCursorPos(&Win32MousePos));
                Assert(ScreenToClient(GlobalState.WindowHandle, &Win32MousePos));

                Win32MousePos.y = ClientRect.bottom - Win32MousePos.y;

                CurrMousePos.x = float(Win32MousePos.x) / float(ClientWidth);
                CurrMousePos.y = float(Win32MousePos.y) / float(ClientHeight);

                MouseDown = (GetKeyState(VK_LBUTTON) & 0x80) != 0;
            }

            if (MouseDown)
            {
                if (!camera->PrevMouseDown)
                {
                    camera->PrevMousePos = CurrMousePos;
                }

                glm::vec2 MouseDelta = CameraRotationSpeed * (CurrMousePos - camera->PrevMousePos);
                camera->Pitch += MouseDelta.y;
                camera->Yaw += MouseDelta.x;

                camera->PrevMousePos = CurrMousePos;
            }

            camera->PrevMouseDown = MouseDown;
            
            glm::mat4 YawTransform = glm::rotate(glm::mat4(1.0f), glm::radians(camera->Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 PitchTransform = glm::rotate(glm::mat4(1.0f), glm::radians(camera->Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 CameraAxisTransform = YawTransform * PitchTransform;



            glm::vec3 Right = glm::normalize((CameraAxisTransform * glm::vec4(1, 0, 0, 0)));
            glm::vec3 Up = glm::normalize((CameraAxisTransform * glm::vec4(0, 1, 0, 0)));
            glm::vec3 LookAt = glm::normalize((CameraAxisTransform * glm::vec4(0, 0, 1, 0)));

            m4 CameraViewTransform = IdentityM4();

            CameraViewTransform.v[0].x = Right.x;
            CameraViewTransform.v[1].x = Right.y;
            CameraViewTransform.v[2].x = Right.z;

            CameraViewTransform.v[0].y = Up.x;
            CameraViewTransform.v[1].y = Up.y;
            CameraViewTransform.v[2].y = Up.z;

            CameraViewTransform.v[0].z = LookAt.x;
            CameraViewTransform.v[1].z = LookAt.y;
            CameraViewTransform.v[2].z = LookAt.z;
            
            if (GlobalState.WDown)
            {
                camera->Pos += LookAt * FrameTime;
            }
            if (GlobalState.SDown)
            {
                camera->Pos -= LookAt * FrameTime;
            }
            if (GlobalState.DDown)
            {
                camera->Pos += Right * FrameTime;
            }
            if (GlobalState.ADown)
            {
                camera->Pos -= Right * FrameTime;
            }

            CameraTransform = CameraViewTransform * TranslationMatrix(-camera->Pos);
        }

        GlobalState.CurrTime = GlobalState.CurrTime + FrameTime;
        if (GlobalState.CurrTime > 2.0f * 3.14159f)
        {
            GlobalState.CurrTime -= 2.0f * 3.14159f;
        }
                        
        float Offset = abs(sin(GlobalState.CurrTime));
        m4 WTransform = (TranslationMatrix(0, 0, 1) *
                         RotationMatrix(0, Pi32 * 0.5f, 0) *
                         ScaleMatrix(1, 1, 1));
        m4 VPTransform = (PerspectiveMatrix(60.0f, AspectRatio, 0.01f, 1000.0f) *
                          CameraTransform);

        dx12_rasterizer* Rasterizer = &GlobalState.Dx12Rasterizer;
        ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

        local_global float Time = 0.0f;
        Time += FrameTime;
        if (Time > 2.0f * Pi32)
        {
            Time -= 2.0f * Pi32;
        }

        point_light_cpu PointLights[NUM_POINT_LIGHTS] = {};
        {
            PointLights[0].Pos = glm::vec3(0, 0.2f + 0.05f * sin(Time), 1);
            PointLights[0].DivisorConstant = 0.4f;
            PointLights[0].Color = 0.3f * glm::vec3(1.0f, 0.3f, 0.3f);

            PointLights[1].Pos = glm::vec3(0.1f * cos(Time), 0.1f, 1.0f + 0.1f * sin(Time));
            PointLights[1].DivisorConstant = 0.4f;
            PointLights[1].Color = 0.2f * glm::vec3(0.0f, 1.0f, 0.3f);

            Dx12CopyDataToBuffer(Rasterizer,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                PointLights,
                sizeof(PointLights),
                Rasterizer->PointLightBuffer);

            for (i32 PlIndex = 0; PlIndex < NUM_POINT_LIGHTS; ++PlIndex)
            {
                m4 PlWTransform = TranslationMatrix(PointLights[PlIndex].Pos) * ScaleMatrix(0.01f, 0.01f, 0.01f);
                Dx12UploadTransformBuffer(Rasterizer,
                    Rasterizer->PlTransformBuffers[PlIndex],
                    PlWTransform,
                    VPTransform,
                    0.001f, 0.0f);
            }
        }

        Dx12UploadTransformBuffer(Rasterizer,
            Rasterizer->SponzaTransformBuffer,
            WTransform,
            VPTransform,
            100.0f, 1.0f);

        {
            dir_light_buffer_cpu LightBufferCopy = {};
            LightBufferCopy.LightAmbientIntensity = 0.4f;
            LightBufferCopy.LightColor = glm::vec3(1.0f, 1.0f, 1.0f);
            LightBufferCopy.LightDirection = glm::normalize(glm::vec3(cos(Time), -1.0f, 0.0f));
            LightBufferCopy.NumPointLights = ArrayCount(PointLights);
            LightBufferCopy.CameraPos = GlobalState.camera.Pos;
            Dx12CopyDataToBuffer(Rasterizer,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                &LightBufferCopy,
                sizeof(LightBufferCopy),
                Rasterizer->DirLightBuffer);
        }

        {
            D3D12_RESOURCE_BARRIER Barrier = {};
            Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            Barrier.Transition.pResource = Rasterizer->FrameBuffers[Rasterizer->CurrentFrameIndex];
            Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            CommandList->ResourceBarrier(1, &Barrier);
        }

        const FLOAT Color[4] = { 1, 0, 1, 1 };
        CommandList->ClearRenderTargetView(Rasterizer->FrameBufferDescriptors[Rasterizer->CurrentFrameIndex], Color, 0, nullptr);
        CommandList->ClearDepthStencilView(Rasterizer->DepthDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, 0);

        CommandList->OMSetRenderTargets(1, Rasterizer->FrameBufferDescriptors + Rasterizer->CurrentFrameIndex, 0,
            &Rasterizer->DepthDescriptor);

        D3D12_VIEWPORT Viewport = {};
        Viewport.TopLeftX = 0;
        Viewport.TopLeftY = 0;
        Viewport.Width = Rasterizer->RenderWidth;
        Viewport.Height = Rasterizer->RenderHeight;
        Viewport.MinDepth = 0;
        Viewport.MaxDepth = 1;
        CommandList->RSSetViewports(1, &Viewport);

        D3D12_RECT ScissorRect = {};
        ScissorRect.left = 0;
        ScissorRect.right = Rasterizer->RenderWidth;
        ScissorRect.top = 0;
        ScissorRect.bottom = Rasterizer->RenderHeight;
        CommandList->RSSetScissorRects(1, &ScissorRect);

        CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        CommandList->SetGraphicsRootSignature(Rasterizer->ModelRootSignature);
        CommandList->SetPipelineState(Rasterizer->ModelPipeline);

        CommandList->SetDescriptorHeaps(1, &Rasterizer->ShaderDescHeap.Heap);
        CommandList->SetGraphicsRootDescriptorTable(1, Rasterizer->LightDescriptor);

        Dx12RenderModel(CommandList, &GlobalState.SponzaModel, Rasterizer->SponzaTransformDescriptor);

        for (i32 PlIndex = 0; PlIndex < NUM_POINT_LIGHTS; ++PlIndex)
        {
            Dx12RenderModel(CommandList, &GlobalState.CubeModel, Rasterizer->PlTransformDescriptors[PlIndex]);
        }

        {
            D3D12_RESOURCE_BARRIER Barrier = {};
            Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            Barrier.Transition.pResource = Rasterizer->FrameBuffers[Rasterizer->CurrentFrameIndex];
            Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            CommandList->ResourceBarrier(1, &Barrier);
        }

        ThrowIfFailed(CommandList->Close());
        Rasterizer->CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&CommandList);
        Rasterizer->SwapChain->Present(1, 0);

        Rasterizer->FenceValue += 1;
        ThrowIfFailed(Rasterizer->CommandQueue->Signal(Rasterizer->Fence, Rasterizer->FenceValue));
        if (Rasterizer->Fence->GetCompletedValue() != Rasterizer->FenceValue)
        {
            // NOTE: Чекаємо аж поки значення у паркані не дорівнює FenceValue
            HANDLE FenceEvent = {};
            ThrowIfFailed(Rasterizer->Fence->SetEventOnCompletion(Rasterizer->FenceValue, FenceEvent));
            WaitForSingleObject(FenceEvent, INFINITE);
        }

        ThrowIfFailed(Rasterizer->CommandAllocator->Reset());
        ThrowIfFailed(CommandList->Reset(Rasterizer->CommandAllocator, 0));

        Rasterizer->CurrentFrameIndex = (Rasterizer->CurrentFrameIndex + 1) % 2;
        Dx12ClearUploadArena(&Rasterizer->UploadArena);
    }
    
    return 0;
}
