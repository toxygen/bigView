#include <assert.h>
#include <Objects/Orientable.h>

Glx::Orientable::Orientable(void)
{
  setDefaults();
}

Glx::Orientable::Orientable(const Glx::Quat& quat)
{
  setOrientation(quat);
}

void 
Glx::Orientable::setOrientation(const Glx::Quat& quat)
{
  itsQuat = quat;
  itsQuat.buildMatrix(itsCurMat);
}

void 
Glx::Orientable::getOrientation(Glx::Quat& quat) const
{
  quat = itsQuat;
}

void 
Glx::Orientable::worldSpaceRotate(Glx::Quat& quat)
{
  itsQuat.worldSpaceRotate(quat);
  itsQuat.buildMatrix(itsCurMat);
}

void 
Glx::Orientable::setDefaults(void)
{
  itsQuat.buildMatrix(itsCurMat);
}
