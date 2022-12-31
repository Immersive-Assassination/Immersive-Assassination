#pragma once

#include <winsock2.h>
#include <random>
#include <unordered_map>
#include <future>

#include <IPluginInterface.h>

#include "Buttplug/Client.hpp"


class ImmersiveAssassination : public IPluginInterface
{
private:
	enum class RatingEventType
	{
		RecordingsRemoved,
		WitnessEliminatedAccident,
		WitnessEliminatedMurder,
		ActorPacified,
		CaughtTrespassing,
		GunshotHeard,
		BulletImpactNoticed,
		UnconsciousBodyFound,
		AlarmTriggered,
		GuardsAlerted,
		DeadBodyFound,
		GuardKilled,
		CaughtOnCamera,
		CaughtCommitingCrime,
		CivilianKilled,
	};
	
	struct RatingEvent
	{
		RatingEventType Type;
		int duration;
		float strength;
		bool enabled;

		ZString TypeToString() const
		{
			switch (Type)
			{
				case RatingEventType::RecordingsRemoved:
					return "RecordingsRemoved";
				case RatingEventType::WitnessEliminatedAccident:
					return "WitnessEliminatedAccident";
				case RatingEventType::WitnessEliminatedMurder:
					return "WitnessEliminatedMurder";
				case RatingEventType::ActorPacified:
					return "ActorPacified";
				case RatingEventType::CaughtTrespassing:
					return "CaughtTrespassing";
				case RatingEventType::GunshotHeard:
					return "GunshotHeard";
				case RatingEventType::BulletImpactNoticed:
					return "BulletImpactNoticed";
				case RatingEventType::UnconsciousBodyFound:
					return "UnconsciousBodyFound";
				case RatingEventType::AlarmTriggered:
					return "AlarmTriggered";
				case RatingEventType::GuardsAlerted:
					return "GuardsAlerted";
				case RatingEventType::DeadBodyFound:
					return "DeadBodyFound";
				case RatingEventType::GuardKilled:
					return "GuardKilled";
				case RatingEventType::CaughtOnCamera:
					return "CaughtOnCamera";
				case RatingEventType::CaughtCommitingCrime:
					return "CaughtCommitingCrime";
				case RatingEventType::CivilianKilled:
					return "CivilianKilled";
			}
			
			return "Unknown";
		}
	
		void DrawSetting();
	};
	
public:
	void Init() override;
	void OnDrawUI(bool p_HasFocus) override;

private:
	void OnEvent(RatingEventType p_EventType);
	void RegisterEvent(RatingEventType p_EventType, int duration, float strength);
	void Reset();

private:
	DEFINE_PLUGIN_DETOUR(ImmersiveAssassination, void, ZGameStatsManager_SendAISignals, ZGameStatsManager* th);
	DEFINE_PLUGIN_DETOUR(ImmersiveAssassination, void, ZAchievementManagerSimple_OnEventSent, ZAchievementManagerSimple* th, uint32_t eventIndex, const ZDynamicObject& event);

private:
	Buttplug::Client* client;
	std::string _address;
	int enabled_device;

	SRWLOCK m_EventLock = {};
	std::unordered_map<RatingEventType, RatingEvent> m_RegisteredEvents;
	std::vector<RatingEvent> m_EventHistory;
};

DEFINE_ZHM_PLUGIN(ImmersiveAssassination)
