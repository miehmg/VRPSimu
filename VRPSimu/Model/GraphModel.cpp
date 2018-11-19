#include "GraphModel.h"

int distance(GraphCoor gr1, GraphCoor gr2)
{
	return gr1.distance(gr2);
}

int distance(Vertice v1, Vertice v2)
{
	return distance(v1.coor, v2.coor);
}

std::ostream &operator<<(std::ostream &os, const PDVertice &v)
{
	os << "x: " << v.coor.xcoor << ", y: " << v.coor.ycoor << ", tw:["
		<< v.strt_time << ", " << v.right_tw << "]";
	return os;
}

std::ostream &operator<<(std::ostream &os, const GraphModel &sg)
{

	for (PDVertice v : sg.vertices)
		os << v << std::endl;
	return os;
}

void InitializePDVertices(GraphModel &sg, const vector<int> &time_vec, const vector<GraphCoor> &pos_seq, const int seed)
{
	if (time_vec.size() < sg.order_size)
	{
		std::cout << "time vector not sufficient" << std::endl;
		return;
	}
	if (time_vec.size() < sg.vehicle_size)
	{
		std::cout << "position vector not sufficient" << std::endl;
		return;
	}
	std::default_random_engine e(seed);
	std::normal_distribution<> n(0, sg.geo_size / 6);
	std::normal_distribution<> m(5, 1);
	for (int i = 0; i != sg.order_size; ++i)
	{
		int x1 = lround(n(e));
		int y1 = lround(n(e));
		int t1 = time_vec[i];
		int x2 = lround(n(e));
		int y2 = lround(n(e));
		int t2 = t1 + m(e);
		GraphCoor gracor1{ x1, y1 };
		GraphCoor gracor2{ x2, y2 };
		sg.vertices.emplace_back(gracor1, t1, t2, PDVertice::isorigin, i, sg.tolerate_time);
		sg.vertices.emplace_back(gracor2, t1, t1, PDVertice::isdest, i, sg.tolerate_time);
		sg.vertices.back().pairver = &(sg.vertices[sg.vertices.size() - 2]);
		sg.vertices[sg.vertices.size() - 2].pairver = &(sg.vertices.back());
	}
	for (int i = 0; i < sg.vehicle_size; ++i)
	{
		sg.vehicle_depots.emplace_back(pos_seq[i], i);
	}
	return;
}