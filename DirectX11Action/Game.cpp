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

	this->CreateGeometry();
	this->CreateVS();
	this->CreateInputLayout();
	this->CreatePS();
}

void Game::Update()
{

}

void Game::Render()
{
	this->RenderBegin();

	// IA - VS - RS - PS - OM
	{
		uint32 stride = sizeof(Vertex);
		uint32 offset = 0;

		// IA
		this->_deviceContext->IASetVertexBuffers(0, 1, this->_vertexBuffer.GetAddressOf(), &stride, &offset);
		this->_deviceContext->IASetInputLayout(this->_inputLayout.Get());
		this->_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// VS
		this->_deviceContext->VSSetShader(this->_vertexShader.Get(), nullptr, 0);

		// RS

		// PS 
		this->_deviceContext->PSSetShader(this->_pixelShader.Get(), nullptr, 0);

		// OM
		this->_deviceContext->Draw(this->_vertices.size(), 0);
	}

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

void Game::CreateGeometry()
{

	// VertexData
	{
		_vertices.resize(3);

		_vertices[0].position = Vec3(-0.5f, -0.5f, 0.f);
		_vertices[0].color = Color(1.f, 0.f, 0.f, 1.f);

		_vertices[1].position = Vec3(0.f, 0.5f, 0.f);
		_vertices[1].color = Color(0.f, 0.f, 1.f, 1.f);

		_vertices[2].position = Vec3(0.5f, -0.5f, 0.f);
		_vertices[2].color = Color(0.f, 1.f, 0.f, 1.f);
	}

	// VertexBuffer
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		{

			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.ByteWidth = (uint32)(sizeof(Vertex) * _vertices.size());
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
		}

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		{
			data.pSysMem = _vertices.data(); // &vertices[0]
		}

		HRESULT hr = this->_device->CreateBuffer(&desc, &data, this->_vertexBuffer.GetAddressOf());
		CHECK(hr);
	}
}

void Game::CreateInputLayout()
{
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			// offsetof(Vertex, position) == 0
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// offsetof(Vertex, color) == 12
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		const int32 count = sizeof(layout) / sizeof(D3D11_INPUT_ELEMENT_DESC);
		HRESULT hr = this->_device->CreateInputLayout(
			layout,
			count, // == _countof(layout)
			this->_vsBlob->GetBufferPointer(),
			this->_vsBlob->GetBufferSize(),
			this->_inputLayout.GetAddressOf()
		);
		CHECK(hr);
	}
}

void Game::CreateVS()
{
	LoadShaderFromFile(L"Default.hlsl", "VS", "vs_5_0", this->_vsBlob);

	HRESULT hr = this->_device->CreateVertexShader(
		this->_vsBlob->GetBufferPointer(),
		this->_vsBlob->GetBufferSize(),
		nullptr,
		this->_vertexShader.GetAddressOf()
	);
	CHECK(hr);
}

void Game::CreatePS()
{
	LoadShaderFromFile(L"Default.hlsl", "PS", "ps_5_0", this->_psBlob);

	HRESULT hr = this->_device->CreatePixelShader(
		this->_psBlob->GetBufferPointer(),
		this->_psBlob->GetBufferSize(),
		nullptr,
		this->_pixelShader.GetAddressOf()
	);
	CHECK(hr);
}

void Game::LoadShaderFromFile(const wstring& path, const string& name, const string& version, ComPtr<ID3DBlob>& blob)
{
	const uint32 compileFlag = D3DCOMPILE_DEBUG| D3DCOMPILE_SKIP_OPTIMIZATION;
	HRESULT hr = ::D3DCompileFromFile(
		path.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		name.c_str(),
		version.c_str(),
		compileFlag,
		0,
		blob.GetAddressOf(),
		nullptr
	);
	CHECK(hr);
}

