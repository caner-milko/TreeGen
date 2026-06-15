#pragma once
#include "TreeWorld.h"
namespace tgen::gen {
class PreviewWorld : public TreeWorld{
public:
	PreviewWorld(TreeWorld& realWorld);
	void ResetToRealWorld();
private:
	TreeWorld& realWorld;
};
}