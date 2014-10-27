/*
 * EdNotFoundHttpController.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: netmind
 */
#include "../ednio_config.h"
#include "EdNotFoundHttpController.h"

namespace edft
{

EdNotFoundHttpController::EdNotFoundHttpController()
{

}

EdNotFoundHttpController::~EdNotFoundHttpController()
{
}

void EdNotFoundHttpController::OnHttpRequestHeader()
{
	mReader.setString("<h1>  Page Not Found......  <h1>\n");
	setRespBodyReader(&mReader,"text/html");
	setHttpResult("404");
}

} /* namespace edft */
