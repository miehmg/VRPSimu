#pragma once

#ifndef SOLUTION_H
#define SOLUTION_H

#include "../common.h"
#include "GraphModel.h"

extern const std::string SOFT_TW;
extern const std::string HARD_TW;

struct Vehicle
{
	vector<int> dec_vrtc;
	set<int> unfin_ord;
	Vertice curr_vrtc;
	TmSpot idl_tm;
	int vehicle_number;

	Vehicle(Vertice curr_vrtc, TmSpot idl_tm, int vehicle_number) : curr_vrtc(curr_vrtc), idl_tm(idl_tm), vehicle_number(vehicle_number) {}
};

struct Move
{
	Vertice orig;
	Vertice dest;
	// Vehicle vhcl;
	int vhcl_num;

	TmSpot last_fins_tm = INT32_MIN;
	TmSpot plan_strt_tm = INT32_MIN;
	TmSpot latst_strt_tm = INT32_MIN;
	TmSpot strt_tm = INT32_MIN;
	TmSpot fins_tm = INT32_MIN;

	Move(Vertice orig, Vertice dest, int vhcl_num) : orig(orig), dest(dest), vhcl_num(vhcl_num) {}
};

struct VerticeEvent
{
	shared_ptr<Vertice> vertice;
	vector<GraphCoor> path;
	TmSpot arrive_time;
	TmSpot compl_time;
	TmSpot leave_time = INT32_MAX;
	int vehicle_number = INT32_MIN;
	int vertice_number = INT32_MIN;

	VerticeEvent(){}
	VerticeEvent(int vn) : vehicle_number(vn) {}
	VerticeEvent(shared_ptr<Vertice> v) : vertice(v)
	{
		if (auto pdv = std::dynamic_pointer_cast<PDVertice>(v))
			vertice_number = pdv->vrtc_no;
	}
};

struct Route
{
	vector<VerticeEvent> move_chain;
	int vhcl_num;
	// vector<int> vertice_num;

	Route(int vn) : vhcl_num(vn) {}
};

struct VerticeState
{
	// VerticeState *pair_vrtc_state;
	PDVertice vertice;
	TmSpot arrive_time = INT32_MIN;
	TmSpot complete_time = INT32_MIN;
	int vehicle_number = INT32_MIN;

	VerticeState(PDVertice &v) : vertice(v) {}
};

struct Solution
{
	GraphModel graph_model;
	vector<Route> vehicle_routes;
	vector<Vehicle> vehicle_states;
	vector<VerticeState> vertice_states;
	int vhcl_cnt;
	TmSpot tm_hrz;

	Solution() {}

	Solution(GraphModel &sg) : graph_model(sg) { vertice_states.reserve(sg.order_size); }

	Solution(GraphModel &sg, vector<Route> &vsg) : graph_model(sg), vehicle_routes(vsg) { vertice_states.reserve(sg.order_size); }
};

Solution EarliestInsertionSolve(GraphModel &);

Solution LatestInsertionSolve(GraphModel &);

Solution NearestInsertionSolve(GraphModel &);

Solution StaticAntSolve(GraphModel &);

// Solution StaticTabuSolve(GraphModel &);

uint32_t DynamicTabuSolve(GraphModel &, Solution &solution, int wait_time);

void FormatRoute(Route &);

TmSpot calculateObjective(Route &, const std::string);
TmSpot softCalculateObjective(Route &);
TmSpot hardCalculateObjective(Route &);

TmSpot calclTimeFeasible(Route &, std::string);
TmSpot softCalclTimeFeasible(Route &);
TmSpot hardCalclTimeFeasible(Route &);

void print(Solution &s);

#endif