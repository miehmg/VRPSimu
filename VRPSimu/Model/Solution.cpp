#pragma once
#include "Solution.h"

const std::string SOFT_TW{ "soft timewindow" };
const std::string HARD_TW{ "hard timewindow" };
const int speed = 300;

bool CheckSequential(vector<int> &pdvseq)
{
	set<int> orders;
	set<int> singorder;
	for (int i = 0; i != pdvseq.size(); ++i)
	{
		if (pdvseq[i] % 2 == 0)
		{
			orders.emplace(pdvseq[i] / 2);
		}
		else
		{
			if (orders.find(pdvseq[i] / 2) == orders.end())
			{
				singorder.emplace(pdvseq[i] / 2);
			}
		}
	}
	set<int> inter;
	std::set_intersection(orders.begin(), orders.end(), singorder.begin(), singorder.end(), std::inserter(inter, inter.begin()));
	if (inter.size() == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int EnumPosition(VerticeEvent lst, vector<shared_ptr<PDVertice>> &pdv, set<int> &rem, vector<int> &seq, vector<int> &tmp, vector<int> &objs)
{
	if (rem.size() == 0)
	{
		vector<int> pdvseq;
		for (int i = 0; i != tmp.size(); ++i)
		{
			int ord = pdv[tmp[i]]->order_no;
			int vrtc = pdv[tmp[i]]->isDest() ? ord * 2 + 1 : ord * 2;
			pdvseq.push_back(vrtc);
		}
		if (!CheckSequential(pdvseq))
		{
			return -1;
		}
		if (objs[0] > lst.compl_time)
		{
			objs[0] = lst.compl_time;
			seq = tmp;
		}
		return 0;
	}
	TmSpot str_tm = lst.compl_time;
	GraphCoor str_coor = lst.vertice->coor;
	int pos = seq.size() - rem.size();
	bool fea = false;

	cout << seq.size() << ' ' << pos;

	// std::default_random_engine e(0);
	// std::uniform_int_distribution<int> u(0, rem.size() - 1);
	// int next = u(e);
	for (int next = 0; next != rem.size(); ++next)
	{
		int x = *std::next(rem.begin(), next);
		PDVertice next_vrtc = *pdv[x];
		TmSpot complete = str_tm + distance(str_coor, next_vrtc.coor) / speed;
		// TmSpot complete = str_tm + distance(*(lst.vertice), next_vrtc) / speed;
		complete = complete > next_vrtc.left_tw ? complete : next_vrtc.left_tw;
		if (complete > next_vrtc.right_tw)
		{
			continue;
		}

		lst.compl_time = complete;
		lst.vertice = std::make_shared<PDVertice>(next_vrtc);
		lst.vertice_number = next_vrtc.vrtc_no;
		tmp[pos] = x;

		set<int> newrem;
		set<int> era{ x };
		std::set_difference(rem.begin(), rem.end(), era.begin(), era.end(), std::inserter(newrem, newrem.begin()));
		if (EnumPosition(lst, pdv, newrem, seq, tmp, objs) != 0)
		{
			continue;
		}
		fea = true;
	}
	return (fea == true ? 0 : -1);
	// for (int i = 0; i != seq.size(); ++i)
	// {
	//     GraphCoor new_coor = seq[i].
	// }
}

void FormatRoute(Route &route, vector<shared_ptr<PDVertice>> &pdv, vector<int> &seq)
{
	for (int i = 1; i != route.move_chain.size(); ++i)
	{
		route.move_chain[i].vertice = pdv[seq[i - 1]];
		route.move_chain[i].vertice_number = pdv[seq[i - 1]]->vrtc_no;
	}

	int cmpl_tm = route.move_chain[0].compl_time;
	cmpl_tm += distance(*route.move_chain[0].vertice, *pdv[seq[0]]) / speed;
	cmpl_tm = cmpl_tm > pdv[seq[0]]->left_tw ? cmpl_tm : pdv[seq[0]]->left_tw;
	VerticeEvent ve{ pdv[seq[0]] };
	ve.vehicle_number = route.vhcl_num;
	ve.compl_time = cmpl_tm;
	route.move_chain[1] = ve;
	for (int i = 1; i != seq.size(); ++i)
	{
		cmpl_tm = cmpl_tm + distance(*pdv[seq[i]], *pdv[seq[i - 1]]) / speed;
		cmpl_tm = cmpl_tm > pdv[seq[i]]->left_tw ? cmpl_tm : pdv[seq[i]]->left_tw;
		route.move_chain[i + 1].compl_time = cmpl_tm;
		// route.move_chain[i + 1].vertice = pdv[seq[i]];
	}
};

int calculateObjective(Route &route, const std::string s)
{

	int size = route.move_chain.size() - 1;
	VerticeEvent start = route.move_chain[0];
	vector<shared_ptr<PDVertice>> pdv;
	for (int i = 1; i != route.move_chain.size(); ++i)
	{
		// PDVertice *vp = dynamic_cast<PDVertice *>(route.move_chain[i].vertice.get());
		shared_ptr<PDVertice> v = std::dynamic_pointer_cast<PDVertice>(route.move_chain[i].vertice);
		pdv.push_back(v);
	}

	// if (size == 2)
	// {
	//     int cmpl = start.compl_time;
	//     int t1 = cmpl + distance(*start.vertice, *pdv[0]) / speed;
	//     cmpl = cmpl > pdv[0]->left_tw ? cmpl : pdv[0]->left_tw;
	//     int t2 = cmpl + distance(*pdv[0], *pdv[1]) / speed;
	//     return t2 - pdv[1]->right_tw;
	// }

	set<int> rem;
	vector<int> seq;
	vector<int> tmp;
	vector<int> objs;
	seq.reserve(size);
	objs.emplace_back(INT32_MAX);
	for (int i = 0; i != size; ++i)
	{
		rem.emplace(i);
	}
	for (int i = 0; i != size; ++i)
	{
		seq.emplace_back(INT32_MIN);
	}

	tmp = seq;
	if (EnumPosition(start, pdv, rem, seq, tmp, objs) == 0)
	{
		FormatRoute(route, pdv, seq);
		int tw = pdv[seq.back()]->right_tw;
		return objs[0] - tw;
	}
	else
	{
		return INT32_MAX;
	}
}

TmSpot calclTimeFeasible(Route &route, std::string s)
{
	TmSpot tm;
	if (s == SOFT_TW)
		tm = softCalclTimeFeasible(route);
	else
		tm = hardCalclTimeFeasible(route);
	return tm;
}

TmSpot hardCalclTimeFeasible(Route &route)
{
	int size = route.move_chain.size() - 1;
	VerticeEvent start = route.move_chain[0];
	vector<shared_ptr<PDVertice>> pdv;
	for (int i = 1; i != route.move_chain.size(); ++i)
	{
		// PDVertice *vp = dynamic_cast<PDVertice *>(route.move_chain[i].vertice.get());
		shared_ptr<PDVertice> v = std::dynamic_pointer_cast<PDVertice>(route.move_chain[i].vertice);
		if (v == nullptr)
		{
			std::cout << "nullptr";
		}
		pdv.push_back(v);
	}

	set<int> rem;
	vector<int> seq;
	vector<int> tmp;
	vector<int> objs;
	seq.reserve(size);
	objs.emplace_back(INT32_MAX);
	for (int i = 0; i != size; ++i)
	{
		rem.emplace(i);
	}
	for (int i = 0; i != size; ++i)
	{
		seq.emplace_back(INT32_MIN);
	}

	tmp = seq;
	if (EnumPosition(start, pdv, rem, seq, tmp, objs) == 0)
	{
		FormatRoute(route, pdv, seq);
		// int tw = pdv[seq.back()]->right_tw;
		return objs[0];
	}
	else
	{
		return INT32_MAX;
	}
};

TmSpot softCalclTimeFeasible(Route &route)
{
	int size = route.move_chain.size() - 1;
	VerticeEvent start = route.move_chain[0];
	vector<shared_ptr<PDVertice>> pdv;
	for (int i = 1; i != route.move_chain.size(); ++i)
	{
		// PDVertice *vp = dynamic_cast<PDVertice *>(route.move_chain[i].vertice.get());
		shared_ptr<PDVertice> v = std::dynamic_pointer_cast<PDVertice>(route.move_chain[i].vertice);
		pdv.push_back(v);
	}

	set<int> rem;
	vector<int> seq;
	vector<int> tmp;
	vector<int> objs;
	seq.reserve(size);
	objs.emplace_back(INT32_MAX);
	for (int i = 0; i != size; ++i)
	{
		rem.emplace(i);
	}
	for (int i = 0; i != size; ++i)
	{
		seq.emplace_back(INT32_MIN);
	}

	tmp = seq;
	if (EnumPosition(start, pdv, rem, seq, tmp, objs) == 0)
	{
		FormatRoute(route, pdv, seq);
		// int tw = pdv[seq.back()]->right_tw;
		return objs[0];
	}
	else
	{
		return INT32_MAX;
	}
};

void print(Solution &s)
{
	vector<Route> vr = s.vehicle_routes;
	std::string ns{ "NULL" };
	for (auto it = vr.cbegin(); it != vr.cend(); ++it)
	{
		cout << it->vhcl_num;
		for (int i = 0; i != 30; ++i)
		{
			cout << '-';
		}
		cout << endl;

		auto veit = (*it).move_chain.cbegin();
		cout << std::left << std::setw(8) << "num";
		cout << std::right << std::setw(8) << it->vhcl_num;
		for (++veit; veit != (*it).move_chain.cend(); ++veit)
		{
			auto pdv = std::dynamic_pointer_cast<PDVertice>(veit->vertice);
			cout << std::right << std::setw(8) << pdv->vrtc_no;
		}
		cout << endl;

		veit = (*it).move_chain.cbegin();
		cout << std::left << std::setw(8) << "xcoor";
		cout << std::right << std::setw(8) << veit->vertice->coor.xcoor;
		for (++veit; veit != (*it).move_chain.cend(); ++veit)
		{
			cout << std::right << std::setw(8) << veit->vertice->coor.xcoor;
		}
		cout << endl;

		veit = (*it).move_chain.cbegin();
		cout << std::left << std::setw(8) << "ycoor";
		cout << std::right << std::setw(8) << veit->vertice->coor.ycoor;
		for (++veit; veit != (*it).move_chain.cend(); ++veit)
		{
			cout << std::right << std::setw(8) << veit->vertice->coor.ycoor;
		}
		cout << endl;

		veit = (*it).move_chain.cbegin();
		cout << std::left << std::setw(8) << "sg_tw";
		cout << std::right << std::setw(8) << veit->compl_time;
		for (++veit; veit != (*it).move_chain.cend(); ++veit)
		{
			auto pdv = std::dynamic_pointer_cast<PDVertice>(veit->vertice);
			if (pdv->isOrigin())
			{
				// cout << std::right << std::setw(8) << s.graph_model.vertices[pdv->vrtc_no].left_tw;
				cout << std::right << std::setw(8) << ns;
			}
			else
			{
				cout << std::right << std::setw(8) << s.graph_model.vertices[pdv->vrtc_no].right_tw;
			}
		}
		cout << endl;

		veit = (*it).move_chain.cbegin();
		cout << std::left << std::setw(8) << "sl_tw";
		cout << std::right << std::setw(8) << veit->compl_time;
		for (++veit; veit != (*it).move_chain.cend(); ++veit)
		{
			auto pdv = std::dynamic_pointer_cast<PDVertice>(veit->vertice);
			// cout << std::right << std::setw(8) << (pdv->isOrigin() ? s.vertice_states[pdv->vrtc_no].vertice.left_tw:s.vertice_states[pdv->vrtc_no].vertice.right_tw);
			if (pdv->isOrigin())
			{
				// cout << std::right << std::setw(8) << s.vertice_states[pdv->vrtc_no].vertice.left_tw;
				cout << std::right << std::setw(8) << ns;
			}
			else
			{
				cout << std::right << std::setw(8) << s.vertice_states[pdv->vrtc_no].vertice.right_tw;
			}
		}
		cout << endl;

		veit = (*it).move_chain.cbegin();
		cout << std::left << std::setw(8) << "cmpl";
		cout << std::right << std::setw(8) << veit->compl_time;
		for (++veit; veit != (*it).move_chain.cend(); ++veit)
		{
			cout << std::right << std::setw(8) << veit->compl_time;
		}
		cout << endl;
	}
}
