#include "pch.h"
#include "Game.h"

Game::Game()
{

}

Game::~Game()
{

}

void Game::Init(HWND hwnd)
{
	this->_hwnd = hwnd;
	this->_width = GWinSizeX;
	this->_height = GWinSizeY;

	this->CreateDeviceAndSwapChain();
	this->CreateRenderTargetView();
	this->SetViewPort();
}

void Game::Update()
{

}

void Game::Render()
{
	this->RenderBegin();

	// TODO

	this->RenderEnd();
}

void Game::RenderBegin()
{
	this->_deviceContext->OMSetRenderTargets(1, this->_renderTargetView.GetAddressOf(), nullptr);
	this->_deviceContext->ClearRenderTargetView(this->_renderTargetView.Get(), this->_clearColor);
	this->_deviceContext->RSSetViewports(1, &this->_viewPort);
}

void Game::RenderEnd()
{
	HRESULT hr = this->_swapChain->Present(1, 0);
	CHECK(hr);
}	

void Game::CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc)); // == memset(&desc, 0, sizeof(desc));
	{
		desc.BufferDesc.Width = this->_width;
		desc.BufferDesc.Height = this->_height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 1;
		desc.OutputWindow = this->_hwnd;
		desc.Windowed = true;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	}

	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&desc,
		this->_swapChain.GetAddressOf(), // **ID3D11SwapChain == &swapChain
		this->_device.GetAddressOf(),
		nullptr,
		this->_deviceContext.GetAddressOf()
	);

	CHECK(hr);
}

void Game::CreateRenderTargetView()
{
	HRESULT hr;

	ComPtr<ID3D11Texture2D> backBuffer;
	hr = this->_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());
	CHECK(hr);

	this->_device->CreateRenderTargetView(backBuffer.Get(), nullptr, this->_renderTargetView.GetAddressOf());
	CHECK(hr);
}

void Game::SetViewPort()
{
	this->_viewPort.TopLeftX = 0.f;
	this->_viewPort.TopLeftY = 0.f;
	this->_viewPort.Width = static_cast<float>(this->_width);
	this->_viewPort.Height = static_cast<float>(this->_height);
	this->_viewPort.MinDepth = 0.f;
	this->_viewPort.MaxDepth = 1.f;
	//this->_deviceContext->RSSetViewports(1, &this->_viewPort);
}
