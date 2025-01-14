#pragma once
#include "Core/UI/UIComponent/UIComponent.h"

#include "Shared/Utilities/EventListener.h"

class Canvas final : public UIComponent 
{
public:
	static std::shared_ptr<Canvas> create_canvas();

	virtual ~Canvas();

private:
	using UIComponent::UIComponent;

	virtual void init();

private:
	DM::Utils::UUID m_resizecallbackUUID = 0;
};