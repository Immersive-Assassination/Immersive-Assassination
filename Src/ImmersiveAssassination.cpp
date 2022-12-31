#include "ImmersiveAssassination.h"

#include "Hooks.h"
#include "Logging.h"
#include "Functions.h"

#include <Glacier/ZScene.h>
#include <Glacier/ZAIGameState.h>
#include <Glacier/SOnlineEvent.h>

#include <nlohmann/json.hpp>
#include <misc/cpp/imgui_stdlib.h>

void ImmersiveAssassination::RatingEvent::DrawSetting()
{
	ImGui::BeginGroup();
		ImGui::Text(this->TypeToString().c_str());
		ImGui::SameLine();
		ImGui::Checkbox("", &this->enabled);
		ImGui::SliderInt("duration", &this->duration, 0, 15000, "%dms");
		ImGui::SliderFloat("strength", &this->strength, 0, 1);
	ImGui::EndGroup();
}

void ImmersiveAssassination::Init()
{
	this->enabled_device = -1;
	this->_address = "ws://127.0.0.1:12345";
	client = new Buttplug::Client("ImmersiveAssassination");

	InitializeSRWLock(&m_EventLock);

	// Register all the rating events. New events should be added here.
	RegisterEvent(RatingEventType::RecordingsRemoved, 500, 0.5);
	RegisterEvent(RatingEventType::WitnessEliminatedAccident, 500, 0.75);
	RegisterEvent(RatingEventType::WitnessEliminatedMurder, 2000, 1.0);
	RegisterEvent(RatingEventType::ActorPacified, 100, 0.5);
	RegisterEvent(RatingEventType::CaughtTrespassing, 4000, 1.0);
	RegisterEvent(RatingEventType::GunshotHeard, 750, 0.25);
	RegisterEvent(RatingEventType::BulletImpactNoticed, 300, 0.5);
	RegisterEvent(RatingEventType::UnconsciousBodyFound, 100, 0.25);
	RegisterEvent(RatingEventType::AlarmTriggered, 3000, 1.0);
	RegisterEvent(RatingEventType::GuardsAlerted, 2000, 0.75);
	RegisterEvent(RatingEventType::DeadBodyFound, 3000, 1.0);
	RegisterEvent(RatingEventType::GuardKilled, 3000, 1.0);
	RegisterEvent(RatingEventType::CaughtOnCamera, 500, 0.3);
	RegisterEvent(RatingEventType::CaughtCommitingCrime, 5000, 0.8);
	RegisterEvent(RatingEventType::CivilianKilled, 8000, 1.0);
	
	Hooks::ZGameStatsManager_SendAISignals01->AddDetour(this, &ImmersiveAssassination::ZGameStatsManager_SendAISignals);
	Hooks::ZGameStatsManager_SendAISignals02->AddDetour(this, &ImmersiveAssassination::ZGameStatsManager_SendAISignals);
	Hooks::ZAchievementManagerSimple_OnEventSent->AddDetour(this, &ImmersiveAssassination::ZAchievementManagerSimple_OnEventSent);
}

void ImmersiveAssassination::OnDrawUI(bool p_HasFocus)
{
	ImGui::PushFont(SDK()->GetImGuiBlackFont());
	const bool s_WindowExpanded = ImGui::Begin("###ImmersiveAssassination", nullptr);
	ImGui::PushFont(SDK()->GetImGuiRegularFont());

	if (s_WindowExpanded)
	{
		if (!client->Connected())
		{
			ImGui::TextColored(ImVec4(0.75, 0, 0, 1), "Not Connected");
			ImGui::InputText("Address", &this->_address);
			if (ImGui::Button("Connect"))
			{
				client->Connect(new Buttplug::WebsocketConnector(this->_address));
			};
		}
		else
		{
			if (ImGui::CollapsingHeader("Devices"))
			{
				ImGui::BeginTable("Devices", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY);
					// None select
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("None");
					ImGui::TableNextColumn();
					ImGui::RadioButton("", &enabled_device, -1);

					// Devices select
					for (auto& it : client->Devices)
					{
						auto dev = it.second;
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", dev->DeviceDisplayName);
						ImGui::TableNextColumn();
						ImGui::RadioButton("", &enabled_device, dev->DeviceIndex);
					}
				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Settings"))
		{
			ImGui::BeginTable("Devices", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY);
				for(auto&& it : m_RegisteredEvents)
				{
					ImGui::TableNextRow();
					it.second.DrawSetting();
				}
			ImGui::EndTable();
		}

		if (ImGui::CollapsingHeader("EventHistory"))
		{
			ImGui::BeginTable("EventHistory", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY);
				AcquireSRWLockShared(&m_EventLock);
				for (auto& s_Event : m_EventHistory)
				{
					auto s_TypeName = s_Event.TypeToString();

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(s_TypeName.c_str(), s_TypeName.c_str() + s_TypeName.size());
				}
				ReleaseSRWLockShared(&m_EventLock);
			ImGui::EndTable();
		}
	}

	ImGui::PopFont();
	ImGui::End();
	ImGui::PopFont();
}

void ImmersiveAssassination::OnEvent(RatingEventType p_EventType)
{
	const auto s_Event = m_RegisteredEvents[p_EventType];

	AcquireSRWLockExclusive(&m_EventLock);
	
	if(enabled_device != -1)
	{
		auto dev = client->Devices.at(enabled_device);
		if(dev->ScalarActuators.size() > 0)
		{
			auto fut = std::async(std::launch::async, [&dev, &s_Event]() {
				auto act = dev->ScalarActuators.at(0);
				act->Command(s_Event.strength);
				std::this_thread::sleep_for(std::chrono::milliseconds(s_Event.duration));
				act->Command(0.0);
			});
		};
	}

	m_EventHistory.push_back(s_Event);

	ReleaseSRWLockExclusive(&m_EventLock);
}

void ImmersiveAssassination::RegisterEvent(RatingEventType p_EventType, int duration, float strength = 1.0)
{
	m_RegisteredEvents[p_EventType] = RatingEvent { p_EventType, duration, strength, true };
}

void ImmersiveAssassination::Reset()
{
	AcquireSRWLockExclusive(&m_EventLock);

	m_EventHistory.clear();
	
	ReleaseSRWLockExclusive(&m_EventLock);
}

DECLARE_PLUGIN_DETOUR(ImmersiveAssassination, void, ZAchievementManagerSimple_OnEventSent, ZAchievementManagerSimple* th, uint32_t eventId, const ZDynamicObject& event)
{
	ZString s_EventData;
	Functions::ZDynamicObject_ToString->Call(const_cast<ZDynamicObject*>(&event), &s_EventData);

	Logger::Debug("Achievement Event Sent: {} - {}", eventId, s_EventData);

	auto s_JsonEvent = nlohmann::json::parse(s_EventData.c_str(), s_EventData.c_str() + s_EventData.size());

	const std::string s_EventName = s_JsonEvent["Name"];
	
	if (s_EventName == "SecuritySystemRecorder")
	{
		if (s_JsonEvent["Value"]["event"] == "spotted")
			OnEvent(RatingEventType::CaughtOnCamera);
		else if (s_JsonEvent["Value"]["event"] == "destroyed")
			OnEvent(RatingEventType::RecordingsRemoved);
	}
	else if (s_EventName == "Kill")
	{
		const bool s_Target = s_JsonEvent["Value"]["IsTarget"];
		const std::string s_ID = s_JsonEvent["Value"]["RepositoryId"];
		const auto s_ActorType = static_cast<EActorType>(s_JsonEvent["Value"]["ActorType"]);

		if (!s_Target && s_ActorType == EActorType::eAT_Civilian)
			OnEvent(RatingEventType::CivilianKilled);

		if (!s_Target && s_ActorType == EActorType::eAT_Guard)
			OnEvent(RatingEventType::GuardKilled);
	}
	else if (s_EventName == "Pacify")
	{
		const bool s_Target = s_JsonEvent["Value"]["IsTarget"];
		const std::string s_ID = s_JsonEvent["Value"]["RepositoryId"];
		const auto s_ActorType = static_cast<EActorType>(s_JsonEvent["Value"]["ActorType"]);

		if (!s_Target)
			OnEvent(RatingEventType::ActorPacified);
	}
	else if (s_EventName == "ContractStart")
	{
		Reset();
	}
	
	return HookResult<void>(HookAction::Continue());
}

DECLARE_PLUGIN_DETOUR(ImmersiveAssassination, void, ZGameStatsManager_SendAISignals, ZGameStatsManager* th)
{
	if (!th->m_oldGameState.m_bHitmanTrespassingSpotted && th->m_gameState.m_bHitmanTrespassingSpotted)
		OnEvent(RatingEventType::CaughtTrespassing);

	if (!th->m_oldGameState.m_bBodyFound && th->m_gameState.m_bBodyFound)
	{
		if(th->m_gameState.m_bBodyFoundPacify) OnEvent(RatingEventType::UnconsciousBodyFound);
		if(th->m_gameState.m_bBodyFoundMurder) OnEvent(RatingEventType::DeadBodyFound);
	}

	if (!th->m_oldGameState.m_bDeadBodySeen && th->m_gameState.m_bDeadBodySeen)
		OnEvent(RatingEventType::DeadBodyFound);

	if (th->m_oldGameState.m_actorCounts.m_nEnemiesIsAlerted == 0 && th->m_gameState.m_actorCounts.m_nEnemiesIsAlerted > 0)
		OnEvent(RatingEventType::GuardsAlerted);

	return HookResult<void>(HookAction::Continue());
}

DECLARE_ZHM_PLUGIN(ImmersiveAssassination);
