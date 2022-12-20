#ifndef FRAMEDATA_H_GUARD
#define FRAMEDATA_H_GUARD

#include <string>
#include <vector>

#include "hitbox.h"
#include "cg.h"

#include <set>
#include <fstream>
#include <cstdint>

extern std::set<int> numberSet;
extern int maxCount;

struct Layer{
	int spriteId = -1;
	bool usePat = true;
	int offset_y;
	int offset_x;
	int blend_mode;
	float rgba[4]{1,1,1,1};
	float rotation[3]{}; //XYZ
	float scale[2]{1,1};//xy
	int priority;
};

template<template<typename> class Allocator = std::allocator>
struct Frame_AF_T {
	// rendering data
	std::vector<Layer, Allocator<Layer>> layers;
	int		duration;
	/* Animation action
	0 (default): End
	1: Next
	2: Jump to frame
	3: Go to start of seq??
	*/
	int aniType; 

	unsigned int aniFlag;

	int jump;
	
	int landJump; //Jumps to this frame if landing.
	//1-5: Linear, Fast end, Slow end, Fast middle, Slow Middle. The last type is not used in vanilla
	int interpolationType; 
	int priority; // Default is 0. Used in throws and dodge.
	int loopCount; //Times to loop, it's the frame just before the loop.
	int loopEnd; //The frame number is not part of the loop.
	
	bool AFRT; //Makes rotation respect EF scale.

	//New, from UNI
	bool afjh;
	uint8_t param[4]; //Let's hope they're right;
	int frameId;

	template<template<typename> class FromT>
	Frame_AF_T<Allocator>& operator=(const Frame_AF_T<FromT>& from) {
		duration = from.duration;
		aniType = from.aniType; 
		aniFlag = from.aniFlag;
		jump = from.jump;
		landJump = from.landJump;
		interpolationType = from.interpolationType; 
		priority = from.priority;
		loopCount = from.loopCount;
		loopEnd = from.loopEnd; 
		AFRT = from.AFRT;
		afjh = from.afjh;
		memcpy(param, from.param, sizeof(param));
		frameId = from.frameId;
		layers.resize(from.layers.size());
		memcpy(layers.data(), from.layers.data(), sizeof(Layer)*from.layers.size()); 
		return *this;
	}
};

struct Frame_AS {
	
	//Acceleration is always set if their corresponding XY speed flags are set.
	//To only set accel, set the add flags with 0 speed. 
	unsigned int movementFlags;
	int speed[2];
	int accel[2];
	int maxSpeedX;

	bool canMove;

	int stanceState;
	int cancelNormal;
	int cancelSpecial;
	int counterType;

	int hitsNumber;
	int invincibility;
	unsigned int statusFlags[2];

	//sinewave thing
	//0 Flags - Similar to ASV0
	//1,2 Distance X,Y
	//3,4 Frames per cycle X,Y
	//5,6 Phases X,Y. Use (0.75, 0) for CCW circles
	unsigned int sineFlags;
	int sineParameters[4];
	float sinePhases[2];
	
	//uni
	int ascf; //related to counterhits or cancel?
};

struct Frame_AT {
	unsigned int guard_flags;
	unsigned int otherFlags;

	int correction;
	//default = 0, is set only if lower
	//1 multiplicative
	//2 substractive
	int correction_type; 

	int damage;
	//int red_damage;
	//int guard_damage;
	int meter_gain;

	//Stand, Air, Crouch
	int guardVector[3];
	int hitVector[3];
	int gVFlags[3];
	int hVFlags[3];

	int hitEffect;
	int soundEffect; //Changes the audio

	int addedEffect; //Lasting visual effect after being hit

	unsigned int hitgrab; //bitfield in uni

	//Affects untech time and launch vector, can be negative.
	float extraGravity;

	int breakTime;
	int untechTime;
	int hitStopTime; //Default value zero. Overrides common values.
	int hitStop; //From common value list
	int blockStopTime; //Needs flag 16 (0 indexed) to be set

	//Uni
	int addHitStun;
	int hitStun;
	int hitStunDecay[3];
	int correction2; //during combo?
	int minDamage;
};

struct Frame_EF {
	int		type;
	int		number;
	int		parameters[12];
};

struct Frame_IF {
	int		type;
	int		parameters[9]; //Max used value is 9. I don't know if parameters beyond have any effect..
};

template<template<typename> class Allocator = std::allocator>
struct Frame_T {
	Frame_AF_T<Allocator> AF = {};
	Frame_AS AS = {};
	Frame_AT AT = {};

	std::vector<Frame_EF,Allocator<Frame_EF>> EF;
	std::vector<Frame_IF,Allocator<Frame_IF>> IF;

	BoxList_T<Allocator> hitboxes{};

	template<template<typename> class FromT>
	Frame_T<Allocator>& operator=(const Frame_T<FromT>& from) {
		AF = from.AF;
		AS = from.AS;
		AT = from.AT;
		EF.resize(from.EF.size());
		memcpy(EF.data(), from.EF.data(), sizeof(Layer)*from.EF.size()); 
		IF.resize(from.IF.size());
		memcpy(IF.data(), from.IF.data(), sizeof(Layer)*from.IF.size()); 

		hitboxes = from.hitboxes;
		return *this;
	};

	//Cast to different allocator.
	template<template<typename> class ToT>
	operator Frame_T<ToT>() const {
		Frame_T<ToT> casted;
		casted.operator=(*this);
		return casted;
	};
};

//Works only for POD vectors.
template<typename Type, typename A, typename B>
void CopyVectorContents(A& dst, const B& src)
{
	static_assert(sizeof(A::value_type) == sizeof(Type) && sizeof(B::value_type) == sizeof(Type), "Vector element types don't match");
	dst.resize(src.size());
	memcpy(dst.data(), src.data(), sizeof(Type)*src.size()); 
}


template<template<typename> class Allocator = std::allocator>
struct Sequence_T {
	// sequence property data
	std::basic_string<char, std::char_traits<char>, Allocator<char>> name;
	std::basic_string<char, std::char_traits<char>, Allocator<char>> codeName;
	
	int psts = 0;
	int level = 0;
	int flag = 0;


	//new
	int pups = 0;

	bool empty = true;
	bool initialized = false;

	std::vector<Frame_T<Allocator>, Allocator<Frame_T<Allocator>>> frames;

	template<template<typename> class FromT>
	Sequence_T<Allocator>& operator=(const Sequence_T<FromT>& from) {
		name.resize(from.name.size());
		memcpy(name.data(), from.name.data(), from.name.size());

		codeName.resize(from.codeName.size());
		memcpy(codeName.data(), from.codeName.data(), from.codeName.size());

		psts = from.psts;
		level = from.level;
		flag = from.flag;
		pups = from.pups;
		empty = from.empty;
		initialized = from.initialized;
		frames.resize(from.frames.size());
		for(int i = 0; i < frames.size(); ++i)
			frames[i] = from.frames[i];
		return *this;
	}
};

class FrameData {
private:
	unsigned int m_nsequences;


public:
	bool m_loaded;
	std::vector<Sequence_T<>> m_sequences;
	void initEmpty();
	bool load(const char *filename, bool patch = false);
	void save(const char *filename);

	int get_sequence_count();

	Sequence_T<>* get_sequence(int n);
	std::string GetDecoratedName(int n);

	void Free();

	FrameData();
	~FrameData();

	void updateSpr(int sprNum);



};

void WriteSequence(std::ofstream &file, const Sequence_T<> *seq);

using Frame = Frame_T<std::allocator>;
using Frame_AF = Frame_AF_T<std::allocator>;
using Sequence = Sequence_T<std::allocator>;

#endif /* FRAMEDATA_H_GUARD */
