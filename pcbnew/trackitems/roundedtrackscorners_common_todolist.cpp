/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) Heikki Pulkkinen.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "roundedtrackscorners.h"
#include <view/view.h>

using namespace TrackNodeItem;


void ROUNDEDTRACKSCORNERS::UpdateListClear(void)
{
    m_update_list->clear();
    m_update_tracks_list->clear();
}

void ROUNDEDTRACKSCORNERS::UpdateListAdd(const ROUNDEDTRACKSCORNER* aCorner)
{
    if(aCorner)
    {
        m_update_list->insert(const_cast<ROUNDEDTRACKSCORNER*>(aCorner));
        
        //Must collect connected tracks corners too.
        TRACK* track = aCorner->GetTrackSeg();
        m_update_tracks_list->insert(static_cast<ROUNDEDCORNERTRACK*>(track));
        
        TRACKNODEITEM* item = Get(track, track->GetStart());
        if(item && dynamic_cast<ROUNDEDTRACKSCORNER*>(item))
            m_update_list->insert(static_cast<ROUNDEDTRACKSCORNER*>(item));
        
        item = Get(track, track->GetEnd());
        if(item && dynamic_cast<ROUNDEDTRACKSCORNER*>(item))
            m_update_list->insert(static_cast<ROUNDEDTRACKSCORNER*>(item));
        
        track = aCorner->GetTrackSegSecond();
        m_update_tracks_list->insert(static_cast<ROUNDEDCORNERTRACK*>(track));

        item = Get(track, track->GetStart());
        if(item && dynamic_cast<ROUNDEDTRACKSCORNER*>(item))
            m_update_list->insert(static_cast<ROUNDEDTRACKSCORNER*>(item));
        
        item = Get(track, track->GetEnd());
        if(item && dynamic_cast<ROUNDEDTRACKSCORNER*>(item))
            m_update_list->insert(static_cast<ROUNDEDTRACKSCORNER*>(item));
    }
}

void ROUNDEDTRACKSCORNERS::UpdateListAdd(const TRACK* aTrackSegFrom)
{
    if(aTrackSegFrom)
    {

        if(aTrackSegFrom->Type() == PCB_ROUNDEDTRACKSCORNER_T)
            UpdateListAdd(static_cast<ROUNDEDTRACKSCORNER*>(const_cast<TRACK*>(aTrackSegFrom))); 
        
        if(dynamic_cast<ROUNDEDCORNERTRACK*>(const_cast<TRACK*>(aTrackSegFrom)))
        {
            m_update_tracks_list->insert(static_cast<ROUNDEDCORNERTRACK*>(const_cast<TRACK*>(aTrackSegFrom)));
            
            TRACKNODEITEM* item = Next(aTrackSegFrom);
            if(item && dynamic_cast<ROUNDEDTRACKSCORNER*>(item))
                UpdateListAdd(static_cast<ROUNDEDTRACKSCORNER*>(item)); 

            item = Back(aTrackSegFrom);
            if(item && dynamic_cast<ROUNDEDTRACKSCORNER*>(item))
                UpdateListAdd(static_cast<ROUNDEDTRACKSCORNER*>(item)); 
        }
    }
}

void ROUNDEDTRACKSCORNERS::UpdateListAdd(const BOARD_ITEM* aBoardItem)
{
    if(dynamic_cast<TRACK*>(const_cast<BOARD_ITEM*>(aBoardItem)))
        if(dynamic_cast<ROUNDEDCORNERTRACK*>(static_cast<BOARD_ITEM*>(const_cast<BOARD_ITEM*>(aBoardItem))))
            UpdateListAdd(static_cast<ROUNDEDCORNERTRACK*>(const_cast<BOARD_ITEM*>(aBoardItem)));
}
                
void ROUNDEDTRACKSCORNERS::UpdateList_DrawTracks(EDA_DRAW_PANEL* aPanel, wxDC* aDC, GR_DRAWMODE aDrawMode)
{
    if(m_update_tracks_list)
        for(auto r_t: *m_update_tracks_list )
            if(r_t)
                r_t->Draw( aPanel, aDC, aDrawMode );
}

void ROUNDEDTRACKSCORNERS::UpdateListDo(void)
{
    if(m_update_tracks_list)
        for(auto r_t: *m_update_tracks_list )
            r_t->ResetVisibleEndpoints();
        
    if(m_update_list)
    {
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
            if(corner)
                corner->ResetVisibleEndpoints();
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
            if(corner)
            {
                corner->Update();
                if(m_EditFrame && m_EditFrame->IsGalCanvasActive())
                    m_EditFrame->GetGalCanvas()->GetView()->Update(corner);
            }
    }
}

void ROUNDEDTRACKSCORNERS::UpdateListDo(EDA_DRAW_PANEL* aPanel, wxDC* aDC, GR_DRAWMODE aDrawMode, bool aErase)
{
    if(m_update_tracks_list)
        for(auto r_t: *m_update_tracks_list )
            r_t->ResetVisibleEndpoints();
    
    if(m_update_list)
    {
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
            if(corner)
                if(aErase)
                    corner->Draw(aPanel, aDC, aDrawMode);
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
            if(corner)
                corner->ResetVisibleEndpoints();
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
            if(corner)
                corner->Update();
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
            if(corner)
                corner->Draw(aPanel, aDC, aDrawMode);
    }
}

void ROUNDEDTRACKSCORNERS::UpdateListDo_UndoRedo(void)
{
    if(m_update_tracks_list)
        for(auto r_t: *m_update_tracks_list )
            r_t->ResetVisibleEndpoints();

    if(m_update_list)
    {
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
            if(corner)
            {
                corner->ConnectTrackSegs();
                corner->ResetVisibleEndpoints();
            }
            
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
        {
            if(corner)
            {
                corner->SetTrackEndpoint();
                corner->SetTrackSecondEndpoint();
                corner->Update();
            }
        }
    }
    
    //GAL
    if( m_EditFrame && m_EditFrame->IsGalCanvasActive() )
    {
        if(m_update_tracks_list)
            for(auto r_t: *m_update_tracks_list )
                m_EditFrame->GetGalCanvas()->GetView()->Update(r_t, KIGFX::GEOMETRY);
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
            if(corner)
                m_EditFrame->GetGalCanvas()->GetView()->Update(corner);
            
    }
}

void ROUNDEDTRACKSCORNERS::UpdateListDo_RemoveBroken(PICKED_ITEMS_LIST* aUndoRedoList)
{
    for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
        if(corner)
            if(!corner->AreTracksConnected())
                Remove(corner, aUndoRedoList, false, true);
        
    UpdateListDo();
}

void ROUNDEDTRACKSCORNERS::UpdateListDo_BlockRotate(PICKED_ITEMS_LIST* aItemsList)
{
    for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
        if(corner)
        {
            for(int n = 0; n < 2; ++n)
            {
                TRACK* trackseg = nullptr;
                n? trackseg = corner->GetTrackSegSecond() : trackseg = corner->GetTrackSeg();
                bool corners_segment = false;
                for( unsigned ii = 0; ii < aItemsList->GetCount(); ii++ )
                {
                    BOARD_ITEM* item = (BOARD_ITEM*) aItemsList->GetPickedItem( ii );
                    if(item && (item->Type() == PCB_TRACE_T))
                        if(static_cast<TRACK*>(item) == trackseg)
                            corners_segment = true;
                }
                if(!corners_segment)
                {
                    Remove(corner, aItemsList, false, true);
            
                }
            }
        }
}

void ROUNDEDTRACKSCORNERS::UpdateListDo_BlockDuplicate(const wxPoint aMoveVector, PICKED_ITEMS_LIST* aUndoRedoList)
{
    if(m_update_list)
    {
        ROUNDEDTRACKSCORNER::PARAMS current_params = GetParams();
        for(ROUNDEDTRACKSCORNER* corner : *m_update_list)
        {
            ROUNDEDTRACKSCORNER::PARAMS params {0, 0, 0};
            if(corner)
            {
                ROUNDEDTRACKSCORNER::PARAMS corner_params = corner->GetParams();
                if(params != corner_params)
                {
                    params = corner_params;
                    SetParams(params);
                }
                
                wxPoint corner_pos = corner->GetEnd() + aMoveVector;
                wxPoint track_start = corner->GetTrackSeg()->GetStart() + aMoveVector;
                wxPoint track_end = corner->GetTrackSeg()->GetEnd() + aMoveVector;
                TRACK* new_track_seg = GetTrackSegment(track_start, track_end, corner->GetLayer(), corner->GetNetCode());
                if(new_track_seg)
                    Add(new_track_seg, corner_pos, aUndoRedoList);
            }
        }
        SetParams(current_params);
        RecreateMenu();
    }
}
