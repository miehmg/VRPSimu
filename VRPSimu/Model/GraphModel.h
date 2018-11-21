#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "../common.h"

struct GraphCoor
{
	int xcoor;
	int ycoor;

	GraphCoor() {}
	GraphCoor(int x, int y) : xcoor(x), ycoor(y) {}
	int distance(GraphCoor d)
	{
		int hd = abs(d.xcoor - xcoor) + abs(d.ycoor - ycoor);
		return abs(hd);
	}
};




struct Vertice
{
	GraphCoor coor;

	Vertice(GraphCoor gr) : coor(gr) {}

	virtual GraphCoor getCoor() { return coor; };

};


struct VrtuVertice : public Vertice
{
	int vhcl_no;

	VrtuVertice(GraphCoor gc, int vn) : Vertice(gc), vhcl_no(vn) {}
};


struct PDVertice : public Vertice
{
	enum vertype
	{
		isorigin,
		isdest
	};
	PDVertice *pairver;
	vertype type;
	int strt_time;              // time order come out or time pickup point is ready
	int left_tw;                // left time window
	int right_tw;               // right time window
	int tolerate_time;
	int order_no;
	int vrtc_no;
	static const TmSpot ifntuppertime = INT32_MAX;

	PDVertice(GraphCoor gr, int stm, int ltw, vertype tp, int on, int tt) : Vertice(gr), strt_time(stm), left_tw(ltw), tolerate_time(tt), type(tp), order_no(on)
	{
		right_tw = isOrigin() ? ifntuppertime : (strt_time + tt);
		vrtc_no = isOrigin() ? order_no * 2 : order_no * 2 + 1;
	}

	bool isOrigin() const { return type == isorigin ? true : false; }
	bool isDest() const { return type == isdest ? true : false; }
};


struct GraphModel
{
	vector<PDVertice> vertices;
	vector<VrtuVertice> vehicle_depots;
	int geo_size;
	int order_size;
	int time_span;
	int tolerate_time;
	int speed = 1;
	int vehicle_size;
	int stride = 20;

	GraphModel() {}

	GraphModel(int gs, int os, int ts, int tt, int spd, int vs = 0)
		: geo_size(gs), order_size(os), time_span(ts), tolerate_time(tt), speed(spd), vehicle_size(vs) {
		vehicle_size = vs > 0 ? vs : os;
	}

	friend void InitializePDVertices(GraphModel &sg, const vector<int> &tm_seq, const vector<GraphCoor> &vhc_strt_pos, const int seed);
};

int distance(Vertice, Vertice);

int distance(GraphCoor, GraphCoor);

std::ostream &operator<<(std::ostream &, const PDVertice &);

std::ostream &operator<<(std::ostream &, const GraphModel &);

void InitializePDVertices(GraphModel &, const vector<int> &, const vector<GraphCoor> &vhc_strt_pos, const int);

#endif