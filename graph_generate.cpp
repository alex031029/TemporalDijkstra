//This file is to generate a graph for temporal private driving query. The input file is BeijingRoad and taxtdataset_plaintxt
#include<stdio.h>
#include<vector>
#include<string>
#include<algorithm>
#include<cmath>
#include"utility1.h"
#include"RTree.h"

#define Pi 3.14159265359
#define EARTH_RADIUS 63781370

using namespace std;

const int VectorDefaultSize=20;

struct Rect
{
  Rect()  {}

  Rect(double a_minX, double a_minY, double a_maxX, double a_maxY)
  {
    min[0] = a_minX;
    min[1] = a_minY;

    max[0] = a_maxX;
    max[1] = a_maxY;
  }


  double min[2];
  double max[2];
};
//node struct
struct node
{
	double longitude;
	double latitude;
	vector<int> out_edges;
	vector<int> in_edges;
	node()
	{
		longitude=-1;
		latitude=-1;
	}
	node(double x, double y)
	{
		longitude=x;
		latitude=y;
	}
	void print()
	{
		printf("longitude:%lf, latitude:%lf\n",longitude,latitude);
	}
};
//edge struct
struct edge
{
	int source;
	int end;
	double length;
	vector<pair<int,int> > speed_info; //timestamp and speed
	vector<pair<int,int> > time_info; //timstamp and transportation time 
        char direction;
	edge()
	{
		source= -1; 
		end= -1;
		length = -1;
		direction = '0';
	}
	edge(int x,int y, double l, char c='0')
	{
		source=x;
		end=y;
		length=l;
		direction = c;
	}
	void speed_info_granule_convert(int gran)
	//it is for coarse granule speed info
	//speed records within gran time interval will be aggregated into one, by taking the medium value
	{	
		vector<pair<int,int> > speed_info_gran; 
		sort(speed_info.begin(),speed_info.end());
		for(int i=0;i<speed_info.size();i++)
		{
			//vector<pair<int,int> > speed_info_temp 
			//it is for storing an array of speed info within gran time interval
			int gran_stop=speed_info[i].first/gran;
			//get which one of granule speed_info[i] belongs to
			int speed_avg=speed_info[i].second;
			//for calculating the average speed for records in the same granule
			int count=1;
			//count the number of records within the same granule
			while(i<speed_info.size())
			{
				//speed_info_temp.push_back(speed_info[i]);
				if(i+1<speed_info.size()&&gran_stop==speed_info[i+1].first/gran)
				{
					i++;
					speed_avg+=speed_info[i].second;
					count++;
				}
				else
				{
					break;
				}
			}
			speed_info_gran.push_back(pair<int,int>(gran_stop*gran,speed_avg/count));
		}
		speed_info=speed_info_gran;
		FIFO_convert();
	}
	bool FIFO()
	{
	//A boolean function check whether f(t) holds FIFO property
		if(time_info.size()<=1)
			return true;
		bool flag=true;
		//printf("u=%d,v=%d\n",source,end);
		for(int i=0;i<time_info.size()-1;i++)
		{
			//getchar();
			if(time_info[i].first+time_info[i].second<time_info[i+1].first+time_info[i+1].second)
				flag=false;
		}
		return flag;
	}
	void FIFO_convert()
	{
		
		//vector<pair<int,int> > speed_info_FIFO; //timestamp and speed satisfying FIFO
		if(speed_info.size()==0)
		{
			time_info.push_back(pair<int,int>(0,(int)(length/10.0)));
			return;
		}
		if(speed_info.size()==1)
		{
			time_info.push_back(pair<int,int>(speed_info[0].first,(int)(length/(double)speed_info[0].second)));
			return;
		}
		//time_info=speed_info;
		pair<int,int> last_time_speed=speed_info[speed_info.size()-1];
		time_info.push_back(pair<int,int>(last_time_speed.first,(int)(length*100.0/(double)last_time_speed.second)));
		for(int i=speed_info.size()-1;i>=0;i--)
		{	
			double time2=length*100.0/(double)speed_info[i+1].second;
			double time1=length*100.0/(double)speed_info[i].second;
			//time_info[i+1].second=(int)time2;
			//time_info[i].second=(int)time1;
			if(speed_info[i].first+time1<speed_info[i+1].first+time2)
			{
				//time_info[i].second=time_info[i+1].second+(time_info[i+1].first-time_info[i].first);
				time1=time2+(speed_info[i+1].first-speed_info[i].first);
			}
			time_info.push_back(pair<int,int>(speed_info[i].first,(int)time1));
		}
		reverse(time_info.begin(),time_info.end());
		//getchar();
		return;
	}
	void speed_info_print(int id,int gran)
	{
		FILE * fout;
		string fname="edge_";
		if(id==-1)
			fname=fname+to_string(source)+"_to_"+to_string(end)+"_granule_"+to_string(gran)+"_speed_info.out";
		else
			
			fname=fname+to_string(id)+"_granule_"+to_string(gran)+"_speed_info.out";

		fout=fopen(fname.c_str(),"w");
		for(int i=0;i<speed_info.size();i++)
		{	
			fprintf(fout,"%d\t%d\n",speed_info[i].first,speed_info[i].second);
		}
	}
	void speed_info_print(FILE * fout)
	{
		for(int i=0;i<speed_info.size();i++)
		{	
			fprintf(fout,"%d,%d,%d,%d\n",source,end,speed_info[i].first,speed_info[i].second);
		}
	}
	void speed_info_print(FILE * fout,int id)
	{
		for(int i=0;i<speed_info.size();i++)
		{	
			fprintf(fout,"%d,%d,%d\n",id,speed_info[i].first,speed_info[i].second);
		}
	}
	void print(int id=-1,int gran=1)
	{
		printf("source:%d,end:%d,length:%lf,speed_info_size:%d\n",source,end,length,speed_info.size());
		speed_info_print(id,gran);		
	}
};
//return true is day is a workday
bool workday_convert(int day)
{
	const int may_first=1; //20090501 is Friday;
	int ret=(day-may_first)%7;  
	return !(ret>=1 and ret<=2);
	
}
//take each minute as the smallest unit for timestamp
int timestamp_convert(int hour, int minute, int second)
{
	return minute+hour*60;
}
//taxi struct for each record in taxi trajectory dataset
struct taxi
{
	int date;
	int timestamp;
	bool workday; // a boolean flag. 1 reprensets it's workday, 0 represents weekend
	double longitude;
	double latitude;
	int direction;
	int speed;
	taxi()
	{
		date=-1;
		timestamp=-1;
		workday=false;
		longitude=-1;
		latitude=-1;
		direction=-1;
		speed=-1;
	}
	taxi(int x, int hour, int minute, int second, double log, double lat, int dir, int spd)
	{
		date=x;
		timestamp=timestamp_convert(hour,minute,second);
		workday=workday_convert(x);
		longitude=log;
		latitude=lat;
		direction=dir;
		speed=spd;
	}
	void print()
	{
		printf("date:%d\ttimestamp:%d\tworkday:%d\tlongtude:%lf\tlatitude:%lf\tdirection:%d\tspeed:%d\n",date,timestamp,workday,longitude,latitude,direction,speed);
	}
};

int n[2]={1285215,426196};
int m[2]={2690296,946434};
//number of nodes and number of edges in BeijingRoad 
iVector<node> node_set(n[0]+n[1]);
iVector<edge> edge_set(m[0]+n[1]);	
iVector<taxi> taxi_set(500000);
//return the direction given two ends of an edge

double rad(double d)
{
	return d*Pi/180.0;
}
double distance(double lat1, double log1, double lat2, double log2) // calculate the distance of an edge
{
	double radLat1=rad(lat1);
	double radLat2=rad(lat2);
	double a=radLat1-radLat2;
	double b=rad(log1)-rad(log2);
	double s=2*sin(sqrt(pow(sin(a/2),2)+cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
	s=s*EARTH_RADIUS;
	return round(s);
}
double distance(edge e) // calculate the distance of an edge
{
	double lat1=node_set[e.source].latitude;
	double log1=node_set[e.source].longitude;
	double lat2=node_set[e.end].latitude;
	double log2=node_set[e.end].longitude;
	double radLat1=rad(lat1);
	double radLat2=rad(lat2);
	double a=radLat1-radLat2;
	double b=rad(log1)-rad(log2);
	double s=2*sin(sqrt(pow(sin(a/2),2)+cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
	s=s*EARTH_RADIUS;
	//printf("%d %d %lf %lf %lf %lf, %lf\n",e.source,e.end,lat1,log1,lat2,log2,s);
	//getchar();
	return round(s);
}
inline char direction_judge(int s, int t)
{
	double y2=node_set[t].longitude,x2=node_set[t].latitude,y1=node_set[s].longitude,x1=node_set[s].latitude;
	double k=(y2-y1)/(x2-x1);
	char c='0';
	if(k>=-1&& k<=1&& x2-x1>=0)
		c='E'; //from west to east
	else if((k>=1|| k<=-1) && y2-y1<=0)
		c='N';
	else if(k>=-1 && k<=1 && x2-x1<=0)
		c='W';
	else 
		c='S';
	return c;
}
inline char direction_judge(edge & e)
{
	if(e.direction=='0')
		e.direction=direction_judge(e.source,e.end);
	return e.direction;
}
//return the direction given the degree representation in taxi dataset
char direction_judge(int direction)
{
	char c='0';
	if(direction>315||direction<=45)
		c='W';
	else if(direction>45 && direction<=135)
		c='N';
	else if(direction>135&& direction<=225)
		c='E';
	else
		c='S';
	return c;
}
char direction_judge(taxi t)
{
	return direction_judge(t.direction);
}
void road_ini()//read nodes and edges from BeijingRoad
{
	string fin_road_file="../BeijingRoad/";
	for(int t=0;t<1;t++)
	{
		string road_edge_file = fin_road_file+"bj"+(char)(t+'1')+".edge.csv";
		string road_node_file = fin_road_file+"bj"+(char)(t+'1')+".node.csv";
		FILE * fin_edge, * fin_node;
		fin_edge=fopen(road_edge_file.c_str(),"r");
		fin_node=fopen(road_node_file.c_str(),"r");
		for(int i=0;i<n[t];i++)
		{
			int id;
			double longitude,latitude;
			fscanf(fin_node,"%d,%lf,%lf",&id,&longitude,&latitude);
			node_set.push_back(node(longitude,latitude));
		}
		for(int i=0;i<m[t];i++)
		{
			int id,source,end;
			double length;
			fscanf(fin_edge,"%d,%d,%d,%lf",&id,&source,&end,&length);
			source--;
			end--;
			int edge_num=i;
			if(t==1)
			{
				source+=n[0];
				end+=n[0];
				edge_num+=m[0];
			}
			edge e=edge(source,end,length);
			e.length=distance(e);
			edge_set.push_back(e);
			node_set[source].out_edges.push_back(edge_num);
			node_set[end].in_edges.push_back(edge_num);
	
	}
		fclose(fin_edge);
		fclose(fin_node);
	}
}
//A function called each time R tree hits the taxi record.And we push the record into edge struct
bool MySearchCallback(int id, void* arg)
{
	int * taxi_id=(int *)arg;
	//printf("%d-th record hits data rect %d\n",taxi_id[0],id);
	taxi t=taxi_set[taxi_id[0]];
	if(direction_judge(edge_set[id])==direction_judge(t))
		edge_set[id].speed_info.push_back(pair<int,int>(t.timestamp,t.speed));
  	//getchar();
	return true; // keep going
}

struct Rect rects[] =
{
	Rect(0.1, 0.1, 2.1, 2.1), // xmin, ymin, xmax, ymax (for 2 dimensional RTree)
    	Rect(5.1, 5.1, 7.1, 7.1),
      	Rect(8.1, 5.1, 9.1, 6.1),
      	Rect(7.1, 1.1, 9.1, 2.1),
};

int nrects = sizeof(rects) / sizeof(rects[0]);
//map each taxi record to at least one edge
inline int taxi_road_mapping()
{
	RTree<int, double, 2, double> tree;

	int i, nhits;
	
	//for(i=0; i<4; i++)
	for(i=0; i<edge_set.size(); i++)
	{
		double log[2]={node_set[edge_set[i].source].longitude, node_set[edge_set[i].end].longitude};
		double lat[2]={node_set[edge_set[i].source].latitude, node_set[edge_set[i].end].latitude};
		double min[2]={Min(log[0],log[1]),Min(lat[0],lat[1])};
		double max[2]={Max(log[0],log[1]),Max(lat[0],lat[1])};
		//printf("%lf,%lf,%lf,%lf",min[0],min[1],max[0],max[1]);
		//getchar();
		tree.Insert(min, max, i); // Note, all values including zero are fine in this version
	}
	/*
	for(i=0; i<nrects; i++)
	{
        	tree.Insert(rects[i].min, rects[i].max, i); // Note, all values including zero are fine in this version
	}
	*/
	//double large_rect_min[]={100,30};
	//double large_rect_max[]={120,50};
	//tree.Insert(large_rect_min,large_rect_max,4);
	/*
	double query_min[2]={101,31};
	double query_max[2]={118,40};
	nhits=tree.Search(query_min, query_max, MySearchCallback, NULL);

	printf("Search results in %d hits\n", nhits); 
	getchar();
	*/
	int epsilon_power = 1;
	//A coefficient initialized to 1. If no road hit for a given taxi record, we double it
	for(int j=0;j<taxi_set.size();j++)
	{
		const double epsilon=0.0001*epsilon_power;
		taxi t = taxi_set[j];
		int speed_threshold=0;
		if(t.speed<=speed_threshold)
			continue;
		double query_min[2]={t.longitude-epsilon,t.latitude-epsilon};
		double query_max[2]={t.longitude+epsilon,t.latitude+epsilon};
		//printf("%lf,%lf,%lf,%lf\n", query_min[0],query_min[1],query_max[0],query_max[1]);
		//getchar();
		//tree.Insert(query_min,query_max,edge_set.size());
		//printf("%d,%d\n",tree.Count(),edge_set.size());
		nhits = tree.Search(query_min, query_max, MySearchCallback, (void *)&j);
		//printf("Search results in %d hits\n", nhits); 
		//getchar();
		if(nhits==0)
		{
			epsilon_power*=2;
			j--;//Increase epsilon until find a road 
		}
		else
			epsilon_power=1;
		if(j%100000==0)
			printf("%d\n",j);
	}
	
}
inline iVector<taxi> taxi_ini()//read taxi trajectory information from taxidataset_plaintxt
{

	string fin_taxi_file="../taxidataset_plaintxt/";
	for(int t=0;t<30;t++)
	{
		if(workday_convert(t+1)==false) //from May 1st. And collect data for workdays/weekends
			continue;
		string date_value=to_string(t+1);
		if(t+1<10)
			date_value="0"+date_value;
		string taxi_file = fin_taxi_file+"output_05"+date_value+".dat.plain";
		printf("%s\n",taxi_file.c_str());
		FILE * fin_taxi=fopen(taxi_file.c_str(),"r");
		int i=0;
		while(1)
		{
			int id,year,month,date,hour,minute,second;
			double longitude,latitude;
			int direction,speed,passenger;
			if(fscanf(fin_taxi,"%d,%d-%d-%d %d:%d:%d,%lf,%lf,%d,%d,%d",&id,&year,&month,&date,&hour,&minute,&second,&latitude,&longitude,&direction,&speed,&passenger)==EOF)
				break;
			//note that the order latitude and longitute is different from BeijingRoad dataset
			//printf("%d\t%d\t%lf\n",id,passenger,latitude);
			//getchar();
			taxi_set.push_back(taxi(date,hour,minute,second,longitude,latitude,direction,speed));
			//if(i++%10000==0)	
				//printf("%d\n",i);
		}
		fclose(fin_taxi);
	}
}
void edge_speed_info_print()   //print speed information for each edge
{
	
	int j=1;
	int partition=edge_set.size()/10;
	for(int i=0;i<edge_set.size();)
	{
		FILE * fout;
		string filename="speed_info_"+to_string(j)+".csv";
		fout=fopen(filename.c_str(),"w");
		while(i%partition!=partition-1&&i<edge_set.size())
		{
			edge_set[i].speed_info_print(fout,i);
			i++;
		}
		i++;
		j++;
		printf("speed_printf %d percent, %d edges completed\n",j*10, i);
		fclose(fout);
	}

}
void temporal_graph_print(int gran=60)
{
	//output the temporal graph
	FILE * fout;
	string filename="../BeijingTemporal.out";
	fout=fopen(filename.c_str(),"w");
	fprintf(fout,"%d\n",node_set.size());

	for(int i=0;i<node_set.size();i++)
	{
		fprintf(fout,"%d ",i);
		for(int j=0;j<node_set[i].out_edges.size();j++)
		{
			edge e=edge_set[node_set[i].out_edges[j]];
			e.speed_info_granule_convert(gran);
			if(e.speed_info.size()>0)
			{
				fprintf(fout,"%d %d ",e.end,e.speed_info.size());
				for(int t=0;t<e.speed_info.size();t++)
				{
					double time=e.length*100.0/(double)e.speed_info[t].second;
					//calcuate the the time function given speed info and length 
					fprintf(fout,"%d %d ", e.speed_info[t].first,(int)time);		
				}
			}
			else
			{
				double time=e.length/10.0;
				fprintf(fout,"%d 1 0 %d ",e.end,(int)time);
			}

		}
		fprintf(fout," -1\n");
	}	
}
double FIFO_percentage()
{
	double ret=0;
	for(int i=0;i<edge_set.size();i++)
	{
		edge e=edge_set[i];
		//getchar();
		if(e.FIFO())
		{
			ret+=1;
			continue;
		}
		/*
		else
		{
			for(int j=0;j<e.speed_info.size();j++)
				printf("%d\t%d\n", e.speed_info[j].first, e.speed_info[j].second);
		}
		getchar();
		*/
	}
	return ret/(double)(edge_set.size());
}

int main()
{
	road_ini();
	taxi_ini();
	//node_set[0].print();
	//taxi_set[0].print();
	//taxi_set[taxi_set.size()-1].print();
	taxi_road_mapping();
	//edge_speed_info_print();
	//edge_set[298444].print();
	//double s1=distance(40.479589,117.362083,40.481161,117.362751);
	//double s2=distance(edge_set[0]);
	//printf("%lf,%lf\n",s1,s2);
	/*
	int id[5]={298444,298422,312466,315630,321700}, gran=60;
	for(int i=0;i<5;i++)
	{
		edge_set[id[i]].speed_info_granule_convert(gran);
		edge_set[id[i]].print(id[i],gran);
	}
	*/
	//getchar();
	temporal_graph_print(60);
	double FIFO_per=FIFO_percentage();
	printf("FIFO_percentage=%lf\n",FIFO_per);
	return 0;
}
