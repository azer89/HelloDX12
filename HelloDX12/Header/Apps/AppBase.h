#ifndef HELLO_DX12_APP_BASE
#define HELLO_DX12_APP_BASE

#include "DX12Helper.h"
#include "Win32Application.h"
#include "PipelineBase.h"
#include "ResourcesBase.h"
#include "Timer.h"
#include "Camera.h"
#include "UIData.h"

#include <vector>
#include <memory>
#include <utility>
#include <type_traits>

class AppBase
{
public:
	AppBase();
	virtual ~AppBase();

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;

	void OnUpdateInternal();
	void BeginRender() const;
	void EndRender();

	void OnKeyDown(uint8_t key);
	void OnKeyUp(uint8_t key);
	void OnMouseMove(int mousePositionX, int mousePositionY);
	void OnMouseLeftPressed();
	void OnMouseLeftRelease();
	void OnKeyboardInput();

	void OnWindowResize(uint32_t width, uint32_t height);

	[[nodiscard]] uint32_t GetWidth() const { return windowWidth_; }
	[[nodiscard]] uint32_t GetHeight() const { return windowHeight_; }
	[[nodiscard]] const WCHAR* GetTitle() const { return title_.c_str(); }

	void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);
	LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	void SetCustomWindowText(const TCHAR* text) const;

	template<class T, class... U>
	requires (std::is_base_of_v<PipelineBase, T>)
	T* AddPipeline(U&&... u)
	{
		// Create std::unique_ptr of Pipeline
		std::unique_ptr<T> pipeline = std::make_unique<T>(std::forward<U>(u)...);
		T* ptr = pipeline.get();
		pipelines_.push_back(std::move(pipeline)); // Put it in std::vector
		return ptr;
	}

	template<class T, class... U>
	requires (std::is_base_of_v<ResourcesBase, T>)
	T* AddResources(U&&... u)
	{
		// Create std::unique_ptr of Resources
		std::unique_ptr<T> resources = std::make_unique<T>(std::forward<U>(u)...);
		T* ptr = resources.get();
		resources_.push_back(std::move(resources)); // Put it in std::vector
		return ptr;
	}

private:
	void ConsoleShow();

protected:
	uint32_t windowWidth_{};
	uint32_t windowHeight_{};
	uint32_t targetWindowWidth_{};
	uint32_t targetWindowHeight_{};
	bool windowResize_{ false };

	float windowAspectRatio_{};

	UIData uiData_{};
	Timer timer_{};

	std::unique_ptr<Camera> camera_{};
	std::vector<std::unique_ptr<PipelineBase>> pipelines_{};
	std::vector<std::unique_ptr<ResourcesBase>> resources_{};

	DX12Context context_{};

private:
	
	// Window title.
	std::wstring title_{};
};

#endif