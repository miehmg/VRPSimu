#include "../Model/Solution.h"

void generateMove(map<int, VerticeState>, map<int, Route> &, map<pair<int, int>, int> &, const vector<int> &, vector<int> &);

// Solution StaticTabuSolve(GraphModel &graph_model)
// {
// }

uint32_t DynamicTabuSolve(GraphModel &graph_model, Solution &solution, int wait_tm)
{
	int wait_seed = 0;
	// vector<PDVertice> &vertices = graph_model.vertices;
	// vector<Vehicle> &vhcl_stats = solution.vehicle_states;
	vector<Route> &vhcl_routes = solution.vehicle_routes;
	vector<VerticeState> &vrtc_stats = solution.vertice_states;
	int& tm_hrz = solution.tm_hrz;
	tm_hrz = INT32_MIN;

	//bool cndt = true;
	//bool ft = true;

	//initialize solution order state
	for (int i = 0; i != graph_model.vertices.size(); ++i)
	{
		vrtc_stats.emplace_back(graph_model.vertices[i]);
	}

	// tm_hrz = vrtc_stats[0].vertice.strt_time + wait_tm;
	while (tm_hrz != INT32_MAX)
	{
		// map<int, Vehicle> tmp_vhcl_stats;
		map<int, Route> tmp_vhcl_routes;
		map<int, VerticeState> tmp_vrtc_stats;
		vector<int> avlb_pdv;

		// find incompleted yet already known vertices
		for (int vrtc_num = 0; vrtc_num != vrtc_stats.size(); ++vrtc_num)
		{
			if (vrtc_stats[vrtc_num].arrive_time == INT32_MIN        // not assigned
				&& vrtc_stats[vrtc_num].vertice.strt_time <= tm_hrz) // already known
			{
				avlb_pdv.push_back(vrtc_num);
				tmp_vrtc_stats.emplace(vrtc_num, vrtc_stats[vrtc_num]); // shallow copy pointer to dec pair
				if (vrtc_stats[vrtc_num].vehicle_number != INT32_MIN && vrtc_stats[vrtc_num].vertice.isDest())
				{
					int vhcl_num = vrtc_stats[vrtc_num].vehicle_number;
					if (tmp_vhcl_routes.find(vhcl_num) == tmp_vhcl_routes.end())
					{
						VerticeEvent lstdest = vhcl_routes[vhcl_num].move_chain.back();
						Route r{ vhcl_num };
						if (lstdest.compl_time < tm_hrz)
						{
							lstdest.compl_time = tm_hrz;
						}
						r.move_chain.push_back(lstdest);

						PDVertice dest = vrtc_stats[vrtc_num].vertice;
						shared_ptr<PDVertice> vp = std::make_shared<PDVertice>(dest);
						VerticeEvent ve{ vp };
						ve.vehicle_number = vhcl_num;
						// ve.vertice_number = dest.vrtc_no;
						r.move_chain.push_back(ve);
						tmp_vhcl_routes.emplace(vhcl_num, r);

						tmp_vrtc_stats.at(vrtc_num).vehicle_number = vhcl_num; //not needed
					}
					else
					{
						PDVertice dest = vrtc_stats[vrtc_num].vertice;
						shared_ptr<PDVertice> vp = std::make_shared<PDVertice>(dest);
						VerticeEvent ve{ vp };
						ve.vehicle_number = vhcl_num;
						tmp_vhcl_routes.at(vhcl_num).move_chain.push_back(ve);
					}
				}
			}
			if (vrtc_stats[vrtc_num].vertice.strt_time > tm_hrz)
			{
				break;
			}
		}

		// generate loose possible vehicle for each order with one order considered
		map<int, vector<int>> avlb_vhcl;
		int next_vhcl_number = vhcl_routes.size();
		for (int i = 0; i != avlb_pdv.size(); ++i)
		{
			vector<int> v;
			int vrtc_num = avlb_pdv[i];
			avlb_vhcl.emplace(vrtc_num, v);
		}
		for (int i = 0; i != avlb_pdv.size(); ++i)
		{
			int vrtc_num = avlb_pdv[i];
			int pair_num = (vrtc_num % 2 == 0) ? vrtc_num + 1 : vrtc_num - 1;
			TmSpot ratm;
			if (vrtc_stats[vrtc_num].vertice.isDest())
				continue;
			for (int j = 0; j != vhcl_routes.size(); ++j)
			{
				Route tmp_r(j);
				if (tmp_vhcl_routes.find(j) == tmp_vhcl_routes.end())
				{
					VerticeEvent lstdest = vhcl_routes[j].move_chain.back();
					lstdest.compl_time = lstdest.compl_time < tm_hrz ? tm_hrz : lstdest.compl_time;
					tmp_r.move_chain.push_back(lstdest);
				}
				else
				{
					tmp_r = tmp_vhcl_routes.at(j);
				}
				shared_ptr<PDVertice> vp1 = std::make_shared<PDVertice>(vrtc_stats[vrtc_num].vertice);
				shared_ptr<PDVertice> vp2 = std::make_shared<PDVertice>(vrtc_stats[pair_num].vertice);
				tmp_r.move_chain.emplace_back(vp1);
				tmp_r.move_chain.emplace_back(vp2);
				ratm = calclTimeFeasible(tmp_r, HARD_TW);
				if (ratm != INT32_MAX)
				{
					// avlb_vhcl.at(vrtc_num).emplace_back(vhcl_num);
					avlb_vhcl.at(vrtc_num).emplace_back(j);
				}
			}
			// if (ratm > 0)
			// {
			//     Route r{vhclsize};
			//     r.move_chain.emplace_back(graph_model.vehicle_depots[vhclsize]);
			//     r.move_chain.front().compl_time = tm_hrz;
			//     r.move_chain.emplace_back(vrtc_stats[vrtc_num].vertice);
			//     r.move_chain.emplace_back(vrtc_stats[pair_num].vertice);
			//     // vhcl_routes.emplace_back(r);

			//     TmSpot newtm = calclTimeFeasible(r, HARD_TW);
			//     if (newtm > 0)
			//     {
			//         tmp_vrtc_stats.at(vrtc_num).vertice.right_tw += newtm > ratm ? ratm : newtm;
			//         vrtc_stats.at(vrtc_num).vertice.right_tw = newtm > ratm ? ratm : newtm;
			//     }
			//     for (int i = 0; i != avlb_pdv.size(); ++i)
			//     {
			//         int vrtc_num = avlb_pdv[i];
			//         avlb_vhcl.at(vrtc_num).emplace_back(vhclsize);
			//     }
			//     ++vhclsize;
			// }
		}

		cout << std::right
			<< std::setw(4) << tm_hrz << std::setw(4) << avlb_pdv.size()
			//  << std::setw(4) << tm_hrz << std::setw(4) << avlb_pdv.size()
			<< endl;
		;

		// generate possible solution
		for (int i = 0; i != avlb_pdv.size(); ++i)
		{
			int vrtc_num = avlb_pdv[i];
			int pair_num = (vrtc_num % 2 == 0) ? vrtc_num + 1 : vrtc_num - 1;
			int vhcl_num = INT32_MIN;
			int min_obj = INT32_MAX;

			if (vrtc_stats[vrtc_num].vertice.isDest())
				continue;

			cout << std::right
				<< std::setw(4) << tm_hrz << std::setw(4) << vrtc_num
				<< std::setw(4) << avlb_vhcl.at(vrtc_num).size()
				<< endl;
			;

			for (int j = 0; j != avlb_vhcl.at(vrtc_num).size(); ++j)
			{

				int tmp_obj;
				int vj = avlb_vhcl.at(vrtc_num)[j];
				Route tmp_r(vj);

				cout << std::right
					//  << "vrtc" << std::setw(4) << vrtc_num
					<< std::setw(4) << vj;
				;

				if (tmp_vhcl_routes.find(vj) == tmp_vhcl_routes.end())
				{
					if (vj >= vhcl_routes.size())
					{
						shared_ptr<VrtuVertice> vp = std::make_shared<VrtuVertice>(graph_model.vehicle_depots[next_vhcl_number]);
						tmp_r.move_chain.emplace_back(vp);
						tmp_r.move_chain.front().compl_time = tm_hrz;
					}
					else
					{
						VerticeEvent lstdest = vhcl_routes[vj].move_chain.back();
						lstdest.compl_time = lstdest.compl_time > tm_hrz ? lstdest.compl_time : tm_hrz;
						tmp_r.move_chain.push_back(lstdest);
					}
				}
				else
				{
					tmp_r = tmp_vhcl_routes.at(vj);
				}
				shared_ptr<PDVertice> vp1 = std::make_shared<PDVertice>(vrtc_stats[vrtc_num].vertice);
				shared_ptr<PDVertice> vp2 = std::make_shared<PDVertice>(vrtc_stats[pair_num].vertice);
				tmp_r.move_chain.emplace_back(vp1);
				tmp_r.move_chain.emplace_back(vp2);
				cout << tmp_r.move_chain.size();
				tmp_obj = calculateObjective(tmp_r, HARD_TW);

				cout << 'f';

				if (min_obj > tmp_obj)
				{
					min_obj = tmp_obj;
					vhcl_num = vj;
					cout << 'm';
				}
				cout << ' ';
			}
			cout << endl << "old finished.  ";
			if (min_obj == INT32_MAX)
			{
				tmp_vhcl_routes.emplace(next_vhcl_number, next_vhcl_number);
				shared_ptr<VrtuVertice> vp = std::make_shared<VrtuVertice>(graph_model.vehicle_depots[next_vhcl_number]);
				tmp_vhcl_routes.at(next_vhcl_number).move_chain.emplace_back(vp);
				tmp_vhcl_routes.at(next_vhcl_number).move_chain.back().compl_time = tm_hrz;
				tmp_vhcl_routes.at(next_vhcl_number).move_chain.back().vehicle_number = i;
				shared_ptr<PDVertice> vp1 = std::make_shared<PDVertice>(vrtc_stats[vrtc_num].vertice);
				shared_ptr<PDVertice> vp2 = std::make_shared<PDVertice>(vrtc_stats[pair_num].vertice);
				tmp_vhcl_routes.at(next_vhcl_number).move_chain.emplace_back(vp1);
				tmp_vhcl_routes.at(next_vhcl_number).move_chain.back().vehicle_number = i;
				tmp_vhcl_routes.at(next_vhcl_number).move_chain.emplace_back(vp2);
				tmp_vhcl_routes.at(next_vhcl_number).move_chain.back().vehicle_number = i;

				TmSpot newtm = calclTimeFeasible(tmp_vhcl_routes.at(next_vhcl_number), HARD_TW);

				cout << "new calculated.  ";
				// Route r = tmp_vhcl_routes.at(next_vhcl_number);
				// TmSpot newtm = calclTimeFeasible(r, HARD_TW);
				if (newtm == INT32_MAX)
				{
					// tmp_vrtc_stats.at(vrtc_num).vertice.right_tw += newtm > ratm ? ratm : newtm;
					// vrtc_stats.at(vrtc_num).vertice.right_tw = newtm > ratm ? ratm : newtm;
					// vrtc_stats[vrtc_num].vertice.right_tw = newtm;
					cout << "ajust tw.  ";
					vp2->right_tw = INT32_MAX;
					newtm = calclTimeFeasible(tmp_vhcl_routes.at(next_vhcl_number), HARD_TW);
					vrtc_stats[pair_num].vertice.right_tw = newtm;
				}

				for (int i = 0; i != avlb_pdv.size(); ++i)
				{
					int vrtc_num = avlb_pdv[i];
					avlb_vhcl.at(vrtc_num).emplace_back(next_vhcl_number);
				}
				++next_vhcl_number;
				cout << endl;
			}
			else
			{
				if (tmp_vhcl_routes.find(vhcl_num) == tmp_vhcl_routes.end())
				{
					tmp_vhcl_routes.emplace(vhcl_num, vhcl_num);
					VerticeEvent lstdest(vhcl_num);
					if (vhcl_num < vhcl_routes.size())
					{
						lstdest = vhcl_routes[vhcl_num].move_chain.back();
						lstdest.compl_time = lstdest.compl_time > tm_hrz ? lstdest.compl_time : tm_hrz;
					}

					else
					{
						lstdest.vertice = std::make_shared<VrtuVertice>(graph_model.vehicle_depots[vhcl_num]);
						lstdest.compl_time = tm_hrz;
					}
					tmp_vhcl_routes.at(vhcl_num).move_chain.push_back(lstdest);
				}
				shared_ptr<PDVertice> vp1 = std::make_shared<PDVertice>(vrtc_stats[vrtc_num].vertice);
				shared_ptr<PDVertice> vp2 = std::make_shared<PDVertice>(vrtc_stats[pair_num].vertice);
				tmp_vhcl_routes.at(vhcl_num).move_chain.emplace_back(vp1);
				tmp_vhcl_routes.at(vhcl_num).move_chain.back().vehicle_number = vhcl_num;
				tmp_vhcl_routes.at(vhcl_num).move_chain.emplace_back(vp2);
				tmp_vhcl_routes.at(vhcl_num).move_chain.back().vehicle_number = vhcl_num;
				// calculateObjective(tmp_vhcl_routes.at(vhcl_num), HARD_TW);
				cout << "old inserted.  " << endl;
			}
		}

		{
			//         bool tabu_cond = true;
			//         // while (tabu_cond)
			//         // {
			//         //     // map<int, Vehicle> tabu_vhcl_stats;
			//         //     map<int, Route> tabu_vhcl_routes = tmp_vhcl_routes;
			//         //     map<int, VerticeState> tabu_vrtc_stats = tmp_vrtc_stats;

			//         //     // map<pair<int, int>, double> pheromones;
			//         //     map<pair<int, int>, int> pheromones;
			//         //     for (int i = 0; i != avlb_pdv.size(); ++i)
			//         //     {
			//         //         int vrtc_num = avlb_pdv[i];
			//         //         for (int j = 0; j != avlb_vhcl.at(i).size(); ++j)
			//         //         {
			//         //             int vhcl_num = avlb_vhcl.at(i)[j];
			//         //             pair<int, int> vv{vrtc_num, vhcl_num};
			//         //             pheromones.emplace(vv, 1);
			//         //         }
			//         //     }

			//         //     vector<int> accs_ver = avlb_pdv;
			//         //     std::random_shuffle(accs_ver.begin(), accs_ver.end());
			//         //     generateMove(tmp_vrtc_stats, tabu_vhcl_routes, pheromones, avlb_pdv, avlb_vhcl, accs_ver);
			// }
		}
		int nextstart = INT32_MAX;
		for (VerticeState vs : vrtc_stats)
		{
			if (vs.vertice.strt_time > tm_hrz)
			{
				TmSpot sttm = vs.vertice.strt_time;
				std::default_random_engine e(wait_seed);
				std::uniform_int_distribution<int> u(0, wait_tm);
				int ram = u(e);
				sttm += ram;
				nextstart = nextstart < sttm ? nextstart : sttm;
				break;
			}
		}
		int nextvrtc = INT32_MIN, nextvhcl = INT32_MIN;
		for (auto it = tmp_vhcl_routes.begin(); it != tmp_vhcl_routes.end(); ++it)
		{
			calculateObjective((*it).second, HARD_TW);
			auto veit = (*it).second.move_chain.begin(); //first vertice event
			++veit;
			auto fpdv = std::dynamic_pointer_cast<PDVertice>((*veit).vertice);

			int remtw = INT32_MAX;
			int nextstr = INT32_MAX;
			int vrtc_num = fpdv->vrtc_no;
			int vhcl_num = (*it).second.vhcl_num;

			if (fpdv->isDest())
			{
				nextstr = vhcl_routes[vhcl_num].move_chain.back().compl_time;
			}
			else
			{
				nextstr = fpdv->strt_time;
				int com = INT32_MIN;
				if (vhcl_routes.size() > vhcl_num && vhcl_routes[vhcl_num].move_chain.size() > 1)
				{
					com = vhcl_routes[vhcl_num].move_chain.back().compl_time;
				}
				nextstr = nextstr > com ? nextstr : com;
			}

			for (; veit != (*it).second.move_chain.end(); ++veit)
			{
				auto pdv = std::dynamic_pointer_cast<PDVertice>((*veit).vertice);

				if (pdv->isOrigin())
					continue;
				int tmp = pdv->right_tw - (*veit).compl_time;
				remtw = remtw < tmp ? remtw : tmp;
			}

			if (remtw < 0)
			{
				remtw = 0;
			}
			std::default_random_engine e(wait_seed);
			std::uniform_int_distribution<int> u(0, (wait_tm > remtw ? remtw : wait_tm));
			// std::uniform_int_distribution<int> u(0, wait_tm);
			// TmSpot lst = tm_hrz + remtw;
			int ran = u(e);
			nextstr += ran;
			// nextstr = nextstr > lst ? lst : nextstr;
			nextstr = nextstr > tm_hrz ? nextstr : tm_hrz;
			// nextstart = nextstart <= nextstr ? (nextstart <= nextstr ? nextstart : nextstr) : nextstart;
			nextvrtc = nextstart <= nextstr ? nextvrtc : vrtc_num;
			nextvhcl = nextstart <= nextstr ? nextvhcl : vhcl_num;
			nextstart = nextstart <= nextstr ? nextstart : nextstr;
		}

		if (nextvrtc != INT32_MIN)
		{
			if (nextvrtc % 2 == 0)
			{
				vrtc_stats[nextvrtc].vehicle_number = nextvhcl;
				vrtc_stats[nextvrtc + 1].vehicle_number = nextvhcl;
			}

			// vrtc_stats[nextvrtc].arrive_time = tmp_vhcl_routes.at(nextvhcl).move_chain[1].arrive_time;
			vrtc_stats[nextvrtc].arrive_time = tmp_vhcl_routes.at(nextvhcl).move_chain[1].compl_time;
			vrtc_stats[nextvrtc].complete_time = tmp_vhcl_routes.at(nextvhcl).move_chain[1].compl_time;

			for (int i = vhcl_routes.size(); i <= nextvhcl; ++i)
			{
				vhcl_routes.emplace_back(i);
				VrtuVertice v = graph_model.vehicle_depots[i];
				shared_ptr<VrtuVertice> vp = std::make_shared<VrtuVertice>(v);
				vhcl_routes[i].move_chain.emplace_back(vp);
				vhcl_routes[i].move_chain.back().compl_time = tm_hrz;
				vhcl_routes[i].move_chain.back().vehicle_number = i;
			}
			vhcl_routes[nextvhcl].move_chain.push_back(tmp_vhcl_routes.at(nextvhcl).move_chain[1]);

			nextstart = nextstart > tm_hrz ? nextstart : tm_hrz;
			// cout << std::right
			//      << std::setw(4) << tm_hrz << std::setw(4) << nextstart
			//      << std::setw(4) << nextvrtc << std::setw(4) << vrtc_stats[nextvrtc].vertice.strt_time
			//      << std::setw(4) << nextvhcl
			//      << std::setw(4) << vhcl_routes[nextvhcl].move_chain.back().compl_time << endl;
		}
		tm_hrz = nextstart;
	}
	return 0;
}
void generateMove(map<int, VerticeState> tabu_vrtc_state, map<int, Route> &tabu_vhcl_routes,
	map<pair<int, int>, int> &pheromones, const vector<int> &avlb_pdv,
	const map<int, vector<int>> avlb_vhcl,
	vector<int> &prio)
{
	// set<int> accs(accs_ver.cbegin(), accs_ver.cend());
	// set<int> whole(avlb_pdv.cbegin(), avlb_pdv.cend());
	// set<int> unaccs;
	// std::set_difference(accs.cbegin(), accs.cend(), whole.cbegin(), whole.cend(), std::inserter(unaccs, unaccs.cbegin()));

	int vrtc_num = prio.back();
	prio.pop_back();
}