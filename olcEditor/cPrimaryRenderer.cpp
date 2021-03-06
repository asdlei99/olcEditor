#include "cPrimaryRenderer.h"

wxDEFINE_EVENT(olcEVT_Editor_MouseMove, cEditorMouseEvent);
wxDEFINE_EVENT(olcEVT_Editor_MouseLeftUp, cEditorMouseEvent);
wxDEFINE_EVENT(olcEVT_Editor_MouseLeftDown, cEditorMouseEvent);
wxDEFINE_EVENT(olcEVT_Editor_MouseRightUp, cEditorMouseEvent);
wxDEFINE_EVENT(olcEVT_Editor_MouseRightDown, cEditorMouseEvent);

cPrimaryRenderer::cPrimaryRenderer(wxWindow* parent, wxGLContext* gl, std::shared_ptr<cGridSelection> gridselect) : cPanAndZoomRenderer(parent, gl)
{
	m_selectionGrid = gridselect;
	gfx.vWorldScale = { 1.0f, 1.0f };
}


cPrimaryRenderer::~cPrimaryRenderer()
{

}


void cPrimaryRenderer::SetArea(std::shared_ptr<cArea> area)
{
	m_area = area;
	Refresh();
}

void cPrimaryRenderer::SetTileSelector(std::shared_ptr<cTileSelection> selector)
{
	m_selectTiles = selector;
}

void cPrimaryRenderer::OnRender()
{
	// Erase everything
	gfx.Clear(olc::PixelF(0.2f, 0.2f, 0.2f, 1.0f ));

	// Draw Canvas in screen space


	// Draw Tile Grid
	olc::vf2d vWorldTL = gfx.ScreenToWorld({ 0,0 });
	olc::vf2d vWorldBR = gfx.ScreenToWorld(gfx.vWorldView * 2.0f);

	// Work out visible world extents
	vWorldTL.x = std::floor(vWorldTL.x);
	vWorldTL.y = std::floor(vWorldTL.y);
	vWorldBR.x = std::ceil(vWorldBR.x);
	vWorldBR.y = std::ceil(vWorldBR.y);

	// Draw Layers
	if (m_area != nullptr)
	{
		// Draw Layers bottom to top
		for (auto& layer : m_area->vecLayers)
		{			
			if (layer->IsVisible())
			{
				if (layer->GetContentType() == ContentType::Tiles)
				{
					// Work out visible tile extents
					olc::vf2d vTileTL = vWorldTL / layer->GetUnitSize();
					olc::vf2d vTileBR = vWorldBR / layer->GetUnitSize();
					vTileTL.x = std::floor(vTileTL.x);
					vTileTL.y = std::floor(vTileTL.y);
					vTileBR.x = std::ceil(vTileBR.x);
					vTileBR.y = std::ceil(vTileBR.y);

					layer->RenderSelf(gfx, vTileTL, vTileBR);

					if (layer == m_selectedLayer)
					{
						layer->RenderBrush(gfx, vTileTL, vTileBR);
					}
				}
			}
		}
	}

	// The actively selected layer will have additional, custom furniture
	if (m_selectedLayer)
	{
		if (m_selectedLayer->GetContentType() == ContentType::Tiles)
		{
			olc::vf2d vTileTL = vWorldTL / m_selectedLayer->GetUnitSize();
			olc::vf2d vTileBR = vWorldBR / m_selectedLayer->GetUnitSize();

			// Draw 0 Axis
			gfx.DrawLine({ 0.0f, vWorldTL.y }, { 0.0f, vWorldBR.y }, olc::PixelF(1, 1, 1, 0.5f), 3);
			gfx.DrawLine({ vWorldTL.x, 0.0f }, { vWorldBR.x, 0.0f }, olc::PixelF(1, 1, 1, 0.5f), 3);

			// Draw Overlay Grid
			for (float x = 0; x <= float(m_selectedLayer->GetLayerSize().x); x++)
			{
				gfx.DrawLine(
					m_selectedLayer->GetUnitSize() * olc::vf2d(x, 0),
					m_selectedLayer->GetUnitSize() * olc::vf2d(x, float(m_selectedLayer->GetLayerSize().y)),
					olc::PixelF(1, 1, 1, 0.2f));
			}

			for (float y = 0; y <= float(m_selectedLayer->GetLayerSize().y); y++)
			{
				gfx.DrawLine(
					m_selectedLayer->GetUnitSize() * olc::vf2d(0, y),
					m_selectedLayer->GetUnitSize() * olc::vf2d(float(m_selectedLayer->GetLayerSize().x), y),
					olc::PixelF(1, 1, 1, 0.2f));
			}

			// Draw Mouse Cursor
			gfx.DrawRect(m_curTile * m_selectedLayer->GetUnitSize(), m_selectedLayer->GetUnitSize(), olc::PixelF(1, 1, 1, 0.5), 6);
			gfx.DrawRect(m_curTile * m_selectedLayer->GetUnitSize(), m_selectedLayer->GetUnitSize(), olc::PixelF(0, 0, 0, 0.5), 3);
		}
	}
}

cEditorMouseEvent cPrimaryRenderer::ConstructMouseEvent(const wxEventTypeTag<cEditorMouseEvent>& e, const olc::vf2d& vWorldPos, const bool bShift, const bool bControl)
{
	cEditorMouseEvent evt(e);
	evt.SetWorld(vWorldPos);
	evt.SetControlHeld(bControl);
	evt.SetShiftHeld(bShift);

	if (m_selectedLayer)
		m_curTile = m_selectedLayer->TileCoord(vWorldPos);
	else
		m_curTile = { 0,0 };
	m_curWorld = vWorldPos;	
	return evt;
}


void cPrimaryRenderer::OnMouseLeftUp(const olc::vf2d& vWorldPos, const bool bShift, const bool bControl)
{
	cEditorMouseEvent e = ConstructMouseEvent(olcEVT_Editor_MouseLeftUp, vWorldPos, bShift, bControl);	
	wxPostEvent(this->GetParent(), e);	
}

void cPrimaryRenderer::OnMouseLeftDown(const olc::vf2d& vWorldPos, const bool bShift, const bool bControl)
{
	cEditorMouseEvent e = ConstructMouseEvent(olcEVT_Editor_MouseLeftDown, vWorldPos, bShift, bControl);
	wxPostEvent(this->GetParent(), e);
}

void cPrimaryRenderer::OnMouseMiddleUp(const olc::vf2d& vWorldPos, const bool bShift, const bool bControl)
{
}

void cPrimaryRenderer::OnMouseMiddleDown(const olc::vf2d& vWorldPos, const bool bShift, const bool bControl)
{
}

void cPrimaryRenderer::OnMouseRightUp(const olc::vf2d& vWorldPos, const bool bShift, const bool bControl)
{
}

void cPrimaryRenderer::OnMouseRightDown(const olc::vf2d& vWorldPos, const bool bShift, const bool bControl)
{
}

void cPrimaryRenderer::OnMouseMove(const olc::vf2d& vWorldPos, const bool bShift, const bool bControl)
{
	cEditorMouseEvent e = ConstructMouseEvent(olcEVT_Editor_MouseMove, vWorldPos, bShift, bControl);
	wxPostEvent(this->GetParent(), e);
}

void cPrimaryRenderer::EnableRegionMode(bool b)
{
	m_bDrawingRegion = b;
}

void cPrimaryRenderer::SetTileRegion(const olc::vi2d& vRegionTL, const olc::vi2d& vRegionBR)
{
	m_vRegionTL = { float(std::min(vRegionTL.x, vRegionBR.x)), float(std::min(vRegionTL.y, vRegionBR.y)) };
	m_vRegionBR = { float(std::max(vRegionTL.x, vRegionBR.x)), float(std::max(vRegionTL.y, vRegionBR.y)) };
}


void cPrimaryRenderer::SetSelectedLayer(std::shared_ptr<cLayer> layer)
{
	m_selectedLayer = layer;
	Refresh();
}

void cPrimaryRenderer::SetSelectedImageResource(std::shared_ptr<cImageResource> image)
{
	m_selectedImageResource = image;
}
