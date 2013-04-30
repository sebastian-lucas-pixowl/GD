/** \file
 *  Game Develop
 *  2008-2013 Florian Rival (Florian.Rival@gmail.com)
 */

#if defined(GD_IDE_ONLY)

#include "RepeatEvent.h"
#include "GDCore/IDE/EventsEditorItemsAreas.h"
#include "GDCore/IDE/EventsEditorSelection.h"
#include "GDCore/IDE/EventsRenderingHelper.h"
#include "GDCore/Events/Serialization.h"
#include "GDL/RuntimeScene.h"
#include "GDL/OpenSaveGame.h"
#include "GDCore/Events/EventsCodeGenerator.h"
#include "GDCore/Events/ExpressionsCodeGeneration.h"
#include "GDCore/Events/EventsCodeGenerationContext.h"
#include "GDL/IDE/Dialogs/EditRepeatEvent.h"

RepeatEvent::RepeatEvent() :
BaseEvent(),
repeatNumberExpression(""),
repeatNumberExpressionSelected(false)
{
}


vector < vector<gd::Instruction>* > RepeatEvent::GetAllConditionsVectors()
{
    vector < vector<gd::Instruction>* > allConditions;
    allConditions.push_back(&conditions);

    return allConditions;
}

vector < vector<gd::Instruction>* > RepeatEvent::GetAllActionsVectors()
{
    vector < vector<gd::Instruction>* > allActions;
    allActions.push_back(&actions);

    return allActions;
}

vector < gd::Expression* > RepeatEvent::GetAllExpressions()
{
    vector < gd::Expression* > allExpressions;
    allExpressions.push_back(&repeatNumberExpression);

    return allExpressions;
}

void RepeatEvent::SaveToXml(TiXmlElement * eventElem) const
{
    TiXmlElement * repeatElem = new TiXmlElement( "RepeatExpression" );
    eventElem->LinkEndChild( repeatElem );
    repeatElem->SetAttribute("value", repeatNumberExpression.GetPlainString().c_str());

    //Les conditions
    TiXmlElement * conditionsElem = new TiXmlElement( "Conditions" );
    eventElem->LinkEndChild( conditionsElem );
    OpenSaveGame::SaveConditions(conditions, conditionsElem);

    //Les actions
    TiXmlElement * actionsElem = new TiXmlElement( "Actions" );
    eventElem->LinkEndChild( actionsElem );
    OpenSaveGame::SaveActions(actions, actionsElem);

    //Sous �v�nements
    if ( !GetSubEvents().empty() )
    {
        TiXmlElement * subeventsElem;
        subeventsElem = new TiXmlElement( "Events" );
        eventElem->LinkEndChild( subeventsElem );

        gd::EventsListSerialization::SaveEventsToXml(events, subeventsElem);
    }
}

void RepeatEvent::LoadFromXml(gd::Project & project, const TiXmlElement * eventElem)
{
    if ( eventElem->FirstChildElement( "RepeatExpression" ) != NULL )
        repeatNumberExpression = gd::Expression(eventElem->FirstChildElement("RepeatExpression")->Attribute("value"));

    //Conditions
    if ( eventElem->FirstChildElement( "Conditions" ) != NULL )
        OpenSaveGame::OpenConditions(conditions, eventElem->FirstChildElement( "Conditions" ));
    else
        cout << "Aucune informations sur les conditions d'un �v�nement";

    //Actions
    if ( eventElem->FirstChildElement( "Actions" ) != NULL )
        OpenSaveGame::OpenActions(actions, eventElem->FirstChildElement( "Actions" ));
    else
        cout << "Aucune informations sur les actions d'un �v�nement";

    //Subevents
    if ( eventElem->FirstChildElement( "Events" ) != NULL )
        gd::EventsListSerialization::LoadEventsFromXml(project, events, eventElem->FirstChildElement( "Events" ));
}

/**
 * Render the event in the bitmap
 */
void RepeatEvent::Render(wxDC & dc, int x, int y, unsigned int width, EventsEditorItemsAreas & areas, EventsEditorSelection & selection, const gd::Platform & platform)
{
    gd::EventsRenderingHelper * renderingHelper = gd::EventsRenderingHelper::GetInstance();
    int border = renderingHelper->instructionsListBorder;
    const int repeatTextHeight = 20;

    //Draw header rectangle
    wxRect headerRect(x, y, width, repeatTextHeight);
    renderingHelper->DrawNiceRectangle(dc, headerRect);

    //Repeat text
    dc.SetFont( renderingHelper->GetNiceFont().Bold() );
    dc.SetTextForeground(wxColour(0,0,0));
    dc.DrawText( _("Repeat") + " " + repeatNumberExpression.GetPlainString() + " " + _("times :"), x + 4, y + 3 );

    //Draw conditions rectangle
    wxRect rect(x, y+repeatTextHeight, renderingHelper->GetConditionsColumnWidth()+border, GetRenderedHeight(width, platform)-repeatTextHeight);
    renderingHelper->DrawNiceRectangle(dc, rect);

    //Draw actions and conditions
    renderingHelper->DrawConditionsList(conditions, dc,
                                        x+border,
                                        y+repeatTextHeight+border,
                                        renderingHelper->GetConditionsColumnWidth()-border, this, areas, selection, platform);
    renderingHelper->DrawActionsList(actions, dc,
                                     x+renderingHelper->GetConditionsColumnWidth()+border,
                                     y+repeatTextHeight+border,
                                     width-renderingHelper->GetConditionsColumnWidth()-border*2, this, areas, selection, platform);
}

unsigned int RepeatEvent::GetRenderedHeight(unsigned int width, const gd::Platform & platform) const
{
    if ( eventHeightNeedUpdate )
    {
        gd::EventsRenderingHelper * renderingHelper = gd::EventsRenderingHelper::GetInstance();
        int border = renderingHelper->instructionsListBorder;
        const int repeatTextHeight = 20;

        //Get maximum height needed
        int conditionsHeight = renderingHelper->GetRenderedConditionsListHeight(conditions, renderingHelper->GetConditionsColumnWidth()-border, platform);
        int actionsHeight = renderingHelper->GetRenderedActionsListHeight(actions, width-renderingHelper->GetConditionsColumnWidth()-border*2, platform);

        renderedHeight = ( conditionsHeight > actionsHeight ? conditionsHeight : actionsHeight ) + repeatTextHeight + border*2;
        eventHeightNeedUpdate = false;
    }

    return renderedHeight;
}

gd::BaseEvent::EditEventReturnType RepeatEvent::EditEvent(wxWindow* parent_, gd::Project & game_, gd::Layout & scene_, gd::MainFrameWrapper & mainFrameWrapper_)
{
    EditRepeatEvent dialog(parent_, *this, game_, scene_);
    if ( dialog.ShowModal() == 0 ) return Cancelled;

    return ChangesMade;
}

/**
 * Initialize from another RepeatEvent.
 * Used by copy ctor and assignement operator
 */
void RepeatEvent::Init(const RepeatEvent & event)
{
    events = CloneVectorOfEvents(event.events);

    conditions = event.conditions;
    actions = event.actions;
    repeatNumberExpression = event.repeatNumberExpression;
}

/**
 * Custom copy operator
 */
RepeatEvent::RepeatEvent(const RepeatEvent & event) :
BaseEvent(event),
repeatNumberExpression("")
{
    Init(event);
}

/**
 * Custom assignement operator
 */
RepeatEvent& RepeatEvent::operator=(const RepeatEvent & event)
{
    if ( this != &event )
    {
        BaseEvent::operator=(event);
        Init(event);
    }

    return *this;
}

#endif
