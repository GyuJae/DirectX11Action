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

	this->CreateRasterizerState();
	this->CreateSamplerState();
	this->CreateBlendState();

	this->CreateSRV();
	this->CreateConstantBuffer();
}

void Game::Update()
{
	D3D11_MAPPED_SUBRESOURCE subResource;
	ZeroMemory(&subResource, sizeof(subResource));
	{
		HRESULT hr = this->_deviceContext->Map(this->_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		CHECK(hr);
		TransformData* p = (TransformData*)subResource.pData;
		p->offset = Vec3(0.f, 0.f, 0.f);
		p->dummy = 0.f;
		this->_deviceContext->Unmap(this->_constantBuffer.Get(), 0);
	}
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
		this->_deviceContext->IASetIndexBuffer(this->_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		this->_deviceContext->IASetInputLayout(this->_inputLayout.Get());
		this->_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// VS
		this->_deviceContext->VSSetShader(this->_vertexShader.Get(), nullptr, 0);
		this->_deviceContext->VSSetConstantBuffers(0, 1, this->_constantBuffer.GetAddressOf());

		// RS
		this->_deviceContext->RSSetState(this->_rasterizerState.Get());

		// PS 
		this->_deviceContext->PSSetShader(this->_pixelShader.Get(), nullptr, 0);
		this->_deviceContext->PSSetShaderResources(0, 1, this->_shaderResourceView.GetAddressOf());
		this->_deviceContext->PSSetSamplers(0, 1, this->_samplerState.GetAddressOf());

		// OM
		//this->_deviceContext->Draw(this->_vertices.size(), 0);
		this->_deviceContext->DrawIndexed(this->_indices.size(), 0, 0);
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
		_vertices.resize(4);

		_vertices[0].position = Vec3(-0.5f, -0.5f, 0.f);
		//_vertices[0].color = Color(1.f, 0.f, 0.f, 1.f);
		_vertices[0].uv = Vec2(0.f, 1.f);

		_vertices[1].position = Vec3(-0.5f, 0.5f, 0.f);
		//_vertices[1].color = Color(1.f, 0.f, 0.f, 1.f);
		_vertices[1].uv = Vec2(0.f, 0.f);

		_vertices[2].position = Vec3(0.5f, -0.5f, 0.f);
		//_vertices[2].color = Color(1.f, 0.f, 0.f, 1.f);
		_vertices[2].uv = Vec2(1.f, 1.f);

		_vertices[3].position = Vec3(0.5f, 0.5f, 0.f);
		//_vertices[3].color = Color(1.f, 0.f, 0.f, 1.f);
		_vertices[3].uv = Vec2(1.f, 0.f);
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

	// IndexData
	{
		_indices = { 0, 1, 2, 2, 1, 3 };
	}

	// IndexBuffer
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		{

			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D10_BIND_INDEX_BUFFER;
			desc.ByteWidth = (uint32)(sizeof(Vertex) * this->_indices.size());
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
		}

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		{
			data.pSysMem = this->_indices.data(); // &_indices[0]
		}

		HRESULT hr = this->_device->CreateBuffer(&desc, &data, this->_indexBuffer.GetAddressOf());
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
			// offsetof(Vertex, uv) == 12
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

void Game::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	{
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
		desc.FrontCounterClockwise = false;
	}
	HRESULT hr = this->_device->CreateRasterizerState(&desc, this->_rasterizerState.GetAddressOf());
	CHECK(hr);
}

void Game::CreateSamplerState()
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MinLOD = -FLT_MAX;
		desc.MaxLOD = FLT_MAX;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	}

	this->_device->CreateSamplerState(&desc, this->_samplerState.GetAddressOf());
}

void Game::CreateBlendState()
{
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	{
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	HRESULT hr = this->_device->CreateBlendState(&desc, this->_blendState.GetAddressOf());
	CHECK(hr);
}

void Game::CreateSRV()
{
	DirectX::TexMetadata md;
	DirectX::ScratchImage img;

	HRESULT hr = ::LoadFromWICFile(
		L"Skeleton.png",
		DirectX::WIC_FLAGS_NONE,
		&md,
		img
	);
	CHECK(hr);

	hr = DirectX::CreateShaderResourceView(
		this->_device.Get(),
		img.GetImages(),
		img.GetImageCount(),
		md,
		this->_shaderResourceView.GetAddressOf()
	);
	CHECK(hr);
}

void Game::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // CPU_Write + GPU_Read
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.ByteWidth = sizeof(TransformData);
	}

	HRESULT hr = this->_device->CreateBuffer(&desc, nullptr, this->_constantBuffer.GetAddressOf());
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

