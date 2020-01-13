
/*******************************************************************************
*  The "New BSD License" : http://www.opensource.org/licenses/bsd-license.php  *
********************************************************************************

Copyright (c) 2010, Mark Turney
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#include "simple_svg.hpp"

using namespace svg;

// Demo page shows sample usage of the Simple SVG library.

int main()
{
    Dimensions dimensions(100, 100);
    Document doc("my_svg.svg", Layout(dimensions, Layout::BottomLeft));

    // Red image border.
    Polygon border(Stroke(1, Color::Red));
    border << Point(0, 0) << Point(dimensions.width, 0)
        << Point(dimensions.width, dimensions.height) << Point(0, dimensions.height);
    doc << border;

    doc << Circle(Point(80, 80), 20, Fill(Color(100, 200, 120)), Stroke(1, Color(200, 250, 150)));

    doc << Text(Point(5, 77), "Simple SVG", Color::Silver, Font(10, "Verdana"));

    doc << (Polygon(Color(200, 160, 220), Stroke(.5, Color(150, 160, 200))) << Point(20, 70)
        << Point(25, 72) << Point(33, 70) << Point(35, 60) << Point(25, 55) << Point(18, 63));

    doc << Rectangle(Point(70, 55), 20, 15, Color::Yellow);

    Container container;
    container << Circle(Point(50, 50), 10, Fill(Color::Aqua))
        << (Container(Fill(), Stroke(1, Color::Green))
            << Line(Point(15, 15), Point(30, 50))
            << Circle(Point(70, 50), 10, Fill(Color::Orange)));

    container.transform(Translation(3, 1.1)).transform(Scaling(1.2));

    doc << container;

    doc.save();
}
