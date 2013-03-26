#include <string>
using std::string;

#include "config.h"

#include "RuleAction.h"
#include "RuleMatcher.h"
#include "Conn.h"
#include "Event.h"
#include "NetVar.h"
#include "PIA.h"

#include "analyzer/Manager.h"

void RuleActionEvent::DoAction(const Rule* parent, RuleEndpointState* state,
				const u_char* data, int len)
	{
	if ( signature_match )
		{
		val_list* vl = new val_list;
		vl->append(rule_matcher->BuildRuleStateValue(parent, state));
		vl->append(new StringVal(msg));

		if ( data )
			vl->append(new StringVal(len, (const char*)data));
		else
			vl->append(new StringVal(""));

		mgr.QueueEvent(signature_match, vl);
		}
	}

void RuleActionEvent::PrintDebug()
	{
	fprintf(stderr, "	RuleActionEvent: |%s|\n", msg);
	}

RuleActionAnalyzer::RuleActionAnalyzer(const char* arg_analyzer)
	{
	string str(arg_analyzer);
	string::size_type pos = str.find(':');
	string arg = str.substr(0, pos);
	analyzer = analyzer_mgr->GetAnalyzerTag(arg);

	if ( ! analyzer )
		reporter->Warning("unknown analyzer '%s' specified in rule", arg.c_str());

	if ( pos != string::npos )
		{
		arg = str.substr(pos + 1);
		child_analyzer = analyzer_mgr->GetAnalyzerTag(arg);

		if ( ! child_analyzer )
			reporter->Warning("unknown analyzer '%s' specified in rule", arg.c_str());
		}
	else
		child_analyzer = analyzer::Tag::ERROR;

	if ( analyzer != analyzer::Tag::ERROR )
		analyzer_mgr->ActivateSigs();
	}

void RuleActionAnalyzer::PrintDebug()
	{
	if ( child_analyzer == analyzer::Tag::ERROR )
		fprintf(stderr, "|%s|\n", analyzer_mgr->GetAnalyzerName(analyzer).c_str());
	else
		fprintf(stderr, "|%s:%s|\n",
			analyzer_mgr->GetAnalyzerName(analyzer).c_str(),
			analyzer_mgr->GetAnalyzerName(child_analyzer).c_str());
	}


void RuleActionEnable::DoAction(const Rule* parent, RuleEndpointState* state,
				const u_char* data, int len)
	{
	if ( ChildAnalyzer() == analyzer::Tag::ERROR )
		{
		if ( ! analyzer_mgr->IsEnabled(Analyzer()) )
			return;

		if ( state->PIA() )
			state->PIA()->ActivateAnalyzer(Analyzer(), parent);
		}
	else
		{
		if ( ! analyzer_mgr->IsEnabled(ChildAnalyzer()) )
			return;

		// This is ugly and works only if there exists only one
		// analyzer of each type.
		state->PIA()->AsAnalyzer()->Conn()->FindAnalyzer(Analyzer())
			->AddChildAnalyzer(ChildAnalyzer());
		}
	}

void RuleActionEnable::PrintDebug()
	{
	fprintf(stderr, "  RuleActionEnable: ");
	RuleActionAnalyzer::PrintDebug();
	}

void RuleActionDisable::DoAction(const Rule* parent, RuleEndpointState* state,
					const u_char* data, int len)
	{
	if ( ChildAnalyzer() == analyzer::Tag::ERROR )
		{
		if ( state->PIA() )
			state->PIA()->DeactivateAnalyzer(Analyzer());
		}
	else
		state->GetAnalyzer()->AddChildAnalyzer(
			state->GetAnalyzer()->FindChild(ChildAnalyzer()));
	}

void RuleActionDisable::PrintDebug()
	{
	fprintf(stderr, "  RuleActionDisable: ");
	RuleActionAnalyzer::PrintDebug();
	}
