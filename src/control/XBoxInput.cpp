#include <eeros/control/XBoxInput.hpp>

using namespace eeros::control;
using namespace eeros::math;

XBoxInput::XBoxInput(std::string dev) {
	setInitPos({0,0,0,0,0,0,0,0});
	j.open(dev.c_str());
	t = new std::thread([this](){ this->j.loop(); });
	axisScale << xScale,      0,      0,      0,
	                  0, yScale,      0,      0,
	                  0,      0, zScale,      0,
	                  0,      0,      0, rScale;
}

XBoxInput::~XBoxInput() {
	delete t;
	j.close();
}

void XBoxInput::run() {
// 	Vector4 v;
// 	
// 	v << j.current.axis[XBoxController::Axis::RY],
// 	     j.current.axis[XBoxController::Axis::RX],
// 	     -j.current.axis[XBoxController::Axis::LY],
// 	     j.current.axis[XBoxController::Axis::RT] - j.current.axis[XBoxController::Axis::LT];
// 	
// 	for(int i = 0; i < v.getNofRows(); i++) {
// 		if(v(i) > -0.2 && v(i) < 0.2) v(i) = 0;
// 	}
// 	
// 	v = out.getSignal().getValue() + axisScale * speedScaleFactor * v;
// 
// 	if (v[0] < min_x)
// 		v[0] = min_x;
// 	else if (v[0] > max_x)
// 		v[0] = max_x;
// 	
// 	if (v[1] < min_y)
// 		v[1] = min_y;
// 	else if (v[1] > max_y)
// 		v[1] = max_y;
// 	
// 	if (v[2] < min_z)
// 		v[2] = min_z;
// 	else if (v[2] > max_z)
// 		v[2] = max_z;
// 	
// 	if (v[3] < min_r)
// 		v[3] = min_r;
// 	else if (v[3] > max_r)
// 		v[3] = max_r;
	
//	out.getSignal().setValue(v);	
	out.getSignal().setValue(Matrix<XBOX_AXIS_COUNT>{
		j.current.axis[XBoxController::Axis::LX],
		j.current.axis[XBoxController::Axis::LY],
		j.current.axis[XBoxController::Axis::LT],
		j.current.axis[XBoxController::Axis::RX],
		j.current.axis[XBoxController::Axis::RY],
		j.current.axis[XBoxController::Axis::RT],
		j.current.axis[XBoxController::Axis::CX],
		j.current.axis[XBoxController::Axis::CY]
	});	
	uint64_t ts = eeros::System::getTimeNs();
	out.getSignal().setTimestamp(ts);
	buttonOut.getSignal().setValue(Matrix<XBOX_BUTTON_COUNT,1,bool>{
		j.current.button_state[XBoxController::Button::A],
		j.current.button_state[XBoxController::Button::B],
		j.current.button_state[XBoxController::Button::X],
		j.current.button_state[XBoxController::Button::Y],
		j.current.button_state[XBoxController::Button::LB],
		j.current.button_state[XBoxController::Button::RB],
		j.current.button_state[XBoxController::Button::back],
		j.current.button_state[XBoxController::Button::start]
	});
	buttonOut.getSignal().setTimestamp(ts);
}

Output<Matrix<XBOX_BUTTON_COUNT,1,bool>>& XBoxInput::getButtonOut() {
	return buttonOut;
}

void XBoxInput::setInitPos(Matrix<XBOX_AXIS_COUNT> initPos) {
	out.getSignal().setValue(initPos);
}

void XBoxInput::setSpeedScaleFactor(double speedScale) {
	if(speedScale >= 0 && speedScale < 5)
		speedScaleFactor = speedScale;
}

