/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include "ChartsExample.h"
#include "ChartsApplication.h"

using namespace Wt;

class ChartsApplication: public WApplication
{
public:
  ChartsApplication(const WEnvironment& env)
    : WApplication(env)
  {
    setTitle("Charts example");

    setCssTheme("polished");
    messageResourceBundle().use(appRoot() + "resources/" + "charts");

    root()->setPadding(10);
    root()->resize(WLength::Auto, WLength::Auto);

    root()->addWidget(cpp14::make_unique<ChartsExample>());

    /*
     * Set our style sheet last, so that it loaded after the ext stylesheets.
     */
    useStyleSheet("resources/charts.css");
  }
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return cpp14::make_unique<ChartsApplication>(env);
}

int chartsApplicationWebServer(int argc, char** argv)
{
  return WRun(argc, argv, &createApplication);
}


//int main(int argc, char **argv)
//{
//  return WRun(argc, argv, &createApplication);
//}
