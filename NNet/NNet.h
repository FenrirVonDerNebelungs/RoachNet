#pragma once
#ifndef NNET_H
#define NNET_H

#ifndef SNET_H
#include "../Nets/sNet.h"
#endif

using namespace std;
class NNet : public Base {
public:
	NNet();
	~NNet();

	unsigned char init(
		float r,
		int N_low_plates,
		int N_nets,
		int N_lev=3,
		int N_lev_trigger=1
	);
	unsigned char init(
		string& inf,
		float r,
		int N_low_plates,
		int N_nets,
		int N_lev=3,
		int N_lev_trigger=1
	);
	void          release();

	unsigned char spawn(s_CNets* nets);/* if file name has been set this will set the weights from the imported file
										   if file name has not been set then the weights will be set based on the 
										   geo metric connections of the eye */
	void          despawn(s_CNets* nets);

	void          setOneHexExDim();/*assumes init has been run, finds approx max dim with slight overage if base of eye is extened by one hex
								     appropriate for seeding on the luna plates*/
	void          setExDim(float rEx);/*assumes init has been run, sets dim to extend the distance of rEx beyond the eye base of these nets*/

	unsigned char setFile(string& inf);
	unsigned char importFile(s_CNets* nets);/* is run during spawn if file name already set, else can be run after spawn
											    import the weight and b values from the file in the nets of CNnets*/
protected:
	/* owned helper objects */
	sNet* m_NetGen;
	HexEye* m_HexEyeGen;
	/*                      */
	string m_file;

	float  m_r;
	int    m_N_plates;
	int    m_N_nets;
	int    m_N_lev;/*number of levels of the nets and the eyes*/
	int    m_N_lev_trigger;/*if this is greater than 0 then there is a trigger node to be set*/

	float m_footprint;/*dimension of the net this spawns at the base of the net, if 0 has not been filled*/

	unsigned char preSetWeights(s_CNets* nets);
	/*helpers to preSetWeights */
	unsigned char preSetWeightNet(s_Net* net, s_HexEye* eye);
};

namespace n_NNet {
	bool run(s_CNets* nets, s_HexPlateLayer* platesIn, s_HexPlateLayer* platesOut, long plate_index);/* assumes all platesIn have the same geometery*/

	/** helpers to run **/
	bool rootNets(s_CNets* nets, s_HexPlateLayer* platesIn, long plate_index);
	void runRootedNets(s_CNets* nets);
}
#endif
