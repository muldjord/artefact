/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            replygenerator.cc
 *
 *  Thu Mar  4 15:39:04 CET 2010
 *  Copyright 2010 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Pentominos.
 *
 *  Pentominos is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Pentominos is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Pentominos; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "replygenerator.h"

/*
<measurements>
<measurement><cpr>000000-0000</cpr><date>20030403</date><instrument>Ark700</instrument><eyes><od>
<unit><capability>KERATOMETRI</capability><subcapability>K1</subcapability><value>08.41</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>K2</subcapability><value>08.25</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>KAXIS</subcapability><value>180</value></unit>
</od><os>
<unit><capability>KERATOMETRI</capability><subcapability>K1</subcapability><value>08.41</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>K2</subcapability><value>08.33</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>KAXIS</subcapability><value>158</value></unit>
<unit><capability>OBJ</capability><subcapability>SPH</subcapability><value>+00.25</value></unit>
<unit><capability>OBJ</capability><subcapability>CYL</subcapability><value>-00.50</value></unit>
<unit><capability>OBJ</capability><subcapability>AXIS</subcapability><value>081</value></unit>
</os></eyes></measurement>

<measurement><cpr>000000-0000</cpr><date>20030403</date><instrument>IOLMASTER</instrument><eyes><od>
<unit><capability>BIOMETRI</capability><subcapability>AXLEN_MAXSNR</subcapability><value>24.6614</value></unit>
<unit><capability>BIOMETRI</capability><subcapability>AXLEN_SNR</subcapability><value>3.8</value></unit>
<unit><capability>BIOMETRI</capability><subcapability>REFINDEX</subcapability><value>1.3549</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>K1</subcapability><value>0.00</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>K2</subcapability><value>3.53</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>KAXIS</subcapability><value>4</value></unit>
</od><os>
<unit><capability>BIOMETRI</capability><subcapability>AXLEN_MAXSNR</subcapability><value>24.7226</value></unit>
<unit><capability>BIOMETRI</capability><subcapability>AXLEN_SNR</subcapability><value>4.0</value></unit>
<unit><capability>BIOMETRI</capability><subcapability>REFINDEX</subcapability><value>1.3549</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>K1</subcapability><value>3.55</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>K2</subcapability><value>3.55</value></unit>
<unit><capability>KERATOMETRI</capability><subcapability>KAXIS</subcapability><value>4</value></unit>
</os></eyes></measurement>

</measurements>
*/

// Reply:
/*
<reply>
	<measurements>
		<measurement>
			<cpr></cpr>
			<date></date>
			<instrument></instrument>
			<eyes>
				<od>
				<unit>
					<capability></capability>
					<subcapability>	</subcapability>
					<value></value>
				</unit>
					.
					.
					.
				</od>
				<os>
				<unit>
					<capability></capability>
					<subcapability>	</subcapability>
					<value></value>
				</unit>
					.
					.
					.
				</os>
			</eyes>
		</measurement>
			.
			.
			.
	</measurements>
		.
		.
		.
</reply>
*/

static std::string generateMeasurement(std::string indent,
                                       atf_result_node_t *root)
{
  std::string meas;

  meas += indent + "  <measurement>\n";
  meas += indent + "    <cpr></cpr>\n";
  meas += indent + "    <date></date>\n";
  meas += indent + "    <instrument></instrument>\n";
  meas += indent + "    <eyes>\n";
  meas += indent + "      <od>\n";
  meas += indent + "        <unit>\n";
  meas += indent + "          <capability></capability>\n";
  meas += indent + "          <subcapability></subcapability>\n";
  meas += indent + "          <value></value>\n";
  meas += indent + "        </unit>\n";
  meas += indent + "      </od>\n";
  meas += indent + "      <os>\n";
  meas += indent + "        <unit>\n";
  meas += indent + "          <capability></capability>\n";
  meas += indent + "          <subcapability></subcapability>\n";
  meas += indent + "          <value></value>\n";
  meas += indent + "        </unit>\n";
  meas += indent + "      </os>\n";
  meas += indent + "    </eyes>\n";
  meas += indent + "  </measurement>\n";

  return meas;
}

std::string generateReply(atf_result_node_t *root)
{
  std::string reply;

  if(root == NULL) return reply;

  reply += "<reply>\n";
  reply += "  <measurements>\n";
  
  struct _atf_result_node_t *node = root->child;
  while(node) {
    reply += generateMeasurement("  ", node);
    node = node->next;
  }

  reply += "  </measurements>\n";
  reply += "</reply>\n";

  return reply;
}

#ifdef TEST_REPLYGENERATOR
//deps:
//cflags:
//libs:
#include "test.h"

TEST_BEGIN;

TEST_END;

#endif/*TEST_REPLYGENERATOR*/
