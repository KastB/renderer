/* Copyright 2011 Malcolm Herring
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * For a copy of the GNU General Public License, see <http://www.gnu.org/licenses/>.
 */

#include "render.h"
#include "rules.h"
#include "s57val.h"

#define d2r(x) (x * M_PI / 180.0)
#define r2d(x) (x * 180.0 / M_PI)

typedef struct HASH Hash_t;
struct HASH {Hash_t *next; char *name; double width; double height;};
Hash_t *hashtable[100];

typedef struct {double x; double y;} XY_t;

const double symbolScale[] = {256.0, 128.0, 64.0, 32.0, 16.0, 8.0, 4.0, 2.0, 1.0,
  0.61, 0.372, 0.227, 0.138, 0.0843, 0.0514, 0.0313, 0.0191, 0.0117, 0.007, 0.138};

const double textScale[] = {256.0, 128.0, 64.0, 32.0, 16.0, 8.0, 4.0, 2.0, 1.0,
  0.5556, 0.3086, 0.1714, 0.0953, 0.0529, 0.0294, 0.0163, 0.0091, 0.0050, 0.0028, 0.0163};

bool bb = false;
double minlat, minlon, maxlat, maxlon, top, mile;
int zoom;
int ref = 0;

Item_t map = {{0,0,0,0,0,0,0,0,0,0}, NULL, NULL, NULL, EMPTY, 0, 0};

Item_t *item = NULL;
Obja_t type = 0;

char string1[1000];
char string2[1000];

typedef struct {char *key; char* val;} Smap_t;

char *body_colours[] = { [COL_UNK]="#000000", [COL_WHT]="#ffffff", [COL_BLK]="#000000", [COL_RED]="#d40000", [COL_GRN]="#00d400",
  [COL_BLU]="blue", [COL_YEL]="#ffd400", [COL_GRY]="grey", [COL_BRN]="brown", [COL_AMB]="#fbf00f", [COL_VIO]="violet",
  [COL_ORG]="orange", [COL_MAG]="#f000f0", [COL_PNK]="pink" };

char *light_colours[] = { [COL_UNK]="magenta", [COL_WHT]="#ffff00", [COL_BLK]="", [COL_RED]="#ff0000", [COL_GRN]="#00ff00", [COL_BLU]="#0000ff", [COL_YEL]="#ffff00",
  [COL_GRY]="", [COL_BRN]="", [COL_AMB]="amber", [COL_VIO]="violet", [COL_ORG]="orange", [COL_MAG]="magenta", [COL_PNK]="" };

char *light_letters[] = { [COL_UNK]="", [COL_WHT]="W", [COL_BLK]="", [COL_RED]="R", [COL_GRN]="G", [COL_BLU]="Bu", [COL_YEL]="Y",
  [COL_GRY]="", [COL_BRN]="", [COL_AMB]="Am", [COL_VIO]="Vi", [COL_ORG]="Or", [COL_MAG]="", [COL_PNK]="" };

char *light_characters[] = { [CHR_UNKN]="", [CHR_F]="F", [CHR_FL]="Fl", [CHR_LFL]="LFl", [CHR_Q]="Q", [CHR_VQ]="VQ", [CHR_UQ]="UQ", [CHR_ISO]="Iso", [CHR_OC]="Oc",
  [CHR_IQ]="IQ", [CHR_IVQ]="IVQ", [CHR_IUQ]="IUQ", [CHR_MO]="Mo", [CHR_FFL]="FFl", [CHR_FLLFL]="FlLFl", [CHR_OCFL]="OcFl", [CHR_FLFL]="FLFl", [CHR_ALOC]="Al.Oc",
  [CHR_ALLFL]="Al.LFl", [CHR_ALFL]="Al.Fl", [CHR_ALGR]="Al.Gr", [CHR_QLFL]="Q+LFl", [CHR_VQLFL]="VQ+LFl", [CHR_UQLFL]="UQ+LFl", [CHR_AL]="Al", [CHR_ALFFL]="Al.FFl" };

char *top_shapes[] = { [TOP_UNKN]="", [TOP_CONE]="top_cone_up", [TOP_ICONE]="top_cone_down", [TOP_SPHR]="top_sphere", [TOP_ISD]="top_isol",
  [TOP_CAN]="top_can", [TOP_BORD]="top_board", [TOP_SALT]="top_saltire", [TOP_CROS]="top_cross", [TOP_CUBE]="", [TOP_WEST]="top_west",
  [TOP_EAST]="top_east", [TOP_RHOM]="top_diamond", [TOP_NORTH]="top_north", [TOP_SOUTH]="top_south", [TOP_BESM]="", [TOP_IBESM]="",
  [TOP_FLAG]="", [TOP_SPRH]="", [TOP_SQUR]="top_square", [TOP_HRECT]="", [TOP_VRECT]="", [TOP_TRAP]="", [TOP_ITRAP]="",
  [TOP_TRI]="top_triangle_up", [TOP_ITRI]="top_triangle_down", [TOP_CIRC]="top_circle", [TOP_CRSS]="", [TOP_T]="", [TOP_TRCL]="",
  [TOP_CRCL]="", [TOP_RHCL]="", [TOP_CLTR]="", [TOP_OTHR]="" };

char *fog_signals[] = { [FOG_UNKN]="", [FOG_EXPL]="Explos", [FOG_DIA]="Dia", [FOG_SIRN]="Siren", [FOG_NAUT]="Horn",
  [FOG_REED]="Horn", [FOG_TYPH]="Horn", [FOG_BELL]="Bell", [FOG_WHIS]="Whis", [FOG_GONG]="Gong", [FOG_HORN]="Horn" };

char *sit_map[] = { [SIT_UNKN]="", [SIT_PRTC]="(Port Control)", [SIT_PRTE]="(Traffic)", [SIT_IPT]="(INT)", [SIT_BRTH]="", [SIT_DOCK]="",
  [SIT_LOCK]="(Lock)", [SIT_FLDB]="", [SIT_BRDG]="(Bridge)", [SIT_DRDG]="", [SIT_TCLT]="(Traffic)" };

char *siw_map[] = { [SIW_UNKN]="", [SIW_DNGR]="(Danger)", [SIW_OBST]="", [SIW_CABL]="", [SIW_MILY]="(Firing)", [SIW_DSTR]="",
  [SIW_WTHR]="(Weather)", [SIW_STRM]="(Storm)", [SIW_ICE]="(Ice)", [SIW_TIME]="(Time)", [SIW_TIDE]="(Tide)", [SIW_TSTR]="(Stream)",
  [SIW_TIDG]="", [SIW_TIDS]="", [SIW_DIVE]="", [SIW_WTLG]="", [SIW_VRCL]="", [SIW_DPTH]="" };

char *rtb_map[] = { [RTB_UNKN]="", [RTB_RAMK]="Racon", [RTB_RACN]="Ramark", [RTB_LDG]="" };

char *scf_map[] = { [SCF_UNKN]="", [SCF_VBTH]="visitor_berth", [SCF_CLUB]="sailing_club", [SCF_BHST]="crane", [SCF_SMKR]="", [SCF_BTYD]="boatyard", [SCF_INN]="",
  [SCF_RSRT]="", [SCF_CHDR]="chandler", [SCF_PROV]="", [SCF_DCTR]="", [SCF_PHRM]="", [SCF_WTRT]="water", [SCF_FUEL]="fuel", [SCF_ELEC]="", [SCF_BGAS]="",
  [SCF_SHWR]="shower", [SCF_LAUN]="laundrette", [SCF_WC]="toilet", [SCF_POST]="", [SCF_TELE]="", [SCF_REFB]="", [SCF_CARP]="", [SCF_BTPK]="", [SCF_CRVN]="",
  [SCF_CAMP]="", [SCF_PMPO]="pump-out", [SCF_EMRT]="", [SCF_SLPW]="slipway", [SCF_VMOR]="visitor_mooring", [SCF_SCRB]="", [SCF_PCNC]="", [SCF_MECH]="", [SCF_SECS]="" };

char *notice_map[] = { [NMK_UNKN]="notice", [NMK_NENT]="notice_a1", [NMK_CLSA]="notice_a1a", [NMK_NOVK]="notice_a2", [NMK_NCOV]="notice_a3", [NMK_NPAS]="notice_a4", [NMK_NBRT]="notice_p",
  [NMK_NBLL]="notice_a5", [NMK_NANK]="notice_anchor", [NMK_NMOR]="notice_bollard", [NMK_NTRN]="notice_turn", [NMK_NWSH]="notice_a9", [NMK_NPSL]="notice_a10a", [NMK_NPSR]="notice_a10b", [NMK_NMTC]="notice_motor",
  [NMK_NSPC]="notice_sport", [NMK_NWSK]="notice_waterski", [NMK_NSLC]="notice_sailboat", [NMK_NUPC]="notice_rowboat", [NMK_NSLB]="notice_sailboard", [NMK_NWBK]="notice_waterbike", [NMK_NHSC]="notice_speedboat",
  [NMK_NLBG]="notice_slipway", [NMK_MVTL]="notice_proceed", [NMK_MVTR]="notice_proceed", [NMK_MVTP]="notice_b2a", [NMK_MVTS]="notice_b2b", [NMK_KPTP]="notice_b3a", [NMK_KPTS]="notice_b3b", [NMK_CSTP]="notice_b4a",
  [NMK_CSTS]="notice_b4b", [NMK_STOP]="notice_b5", [NMK_SPDL]="notice_b6", [NMK_SHRN]="notice_b7", [NMK_KPLO]="notice_b8", [NMK_GWJN]="notice_junction", [NMK_GWCS]="notice_crossing", [NMK_MKRC]="notice_vhf",
  [NMK_LMDP]="notice_c1", [NMK_LMHR]="notice_c2", [NMK_LMWD]="notice_c3", [NMK_NAVR]="notice_c4", [NMK_CHDL]="notice_c5a", [NMK_CHDR]="notice_c5b", [NMK_CHTW]="notice_d1a", [NMK_CHOW]="notice_d1b",
  [NMK_OPTR]="notice_d2a", [NMK_OPTL]="notice_d2b", [NMK_PRTL]="notice_proceed", [NMK_PRTR]="notice_proceed", [NMK_ENTP]="notice_e1", [NMK_OVHC]="notice_e2", [NMK_WEIR]="notice_e3", [NMK_FERN]="notice_e4a",
  [NMK_FERI]="notice_e4b", [NMK_BRTP]="notice_p", [NMK_BTLL]="notice_e5_1", [NMK_BTLS]="notice_e5_2", [NMK_BTRL]="notice_e5_3", [NMK_BTUP]="notice_e5_4", [NMK_BTP1]="notice_e5_5", [NMK_BTP2]="notice_e5_6",
  [NMK_BTP3]="notice_e5_7", [NMK_BTUN]="notice_e5_8", [NMK_BTN1]="notice_e5_9", [NMK_BTN2]="notice_e5_10", [NMK_BTN3]="notice_e5_11", [NMK_BTUM]="notice_e5_12", [NMK_BTU1]="notice_e5_13",
  [NMK_BTU2]="notice_e5_14", [NMK_BTU3]="notice_e5_15", [NMK_ANKP]="notice_anchor", [NMK_MORP]="notice_bollard", [NMK_VLBT]="notice_e7_1", [NMK_TRNA]="notice_turn", [NMK_SWWC]="notice_crossing",
  [NMK_SWWR]="notice_junction", [NMK_SWWL]="notice_junction", [NMK_WRSA]="notice_junction_r", [NMK_WLSA]="notice_junction_l", [NMK_WRSL]="notice_junction_l", [NMK_WLSR]="notice_junction_r",
  [NMK_WRAL]="notice_crossing_r", [NMK_WLAR]="notice_crossing_l", [NMK_MWWC]="notice_crossing", [NMK_MWWJ]="notice_junction", [NMK_MWAR]="notice_junction_l", [NMK_MWAL]="notice_junction_r", [NMK_WARL]="notice_crossing_r",
  [NMK_WALR]="notice_crossing_l", [NMK_PEND]="notice_e11", [NMK_DWTR]="notice_e13", [NMK_TELE]="notice_e14", [NMK_MTCP]="notice_motor", [NMK_SPCP]="notice_sport", [NMK_WSKP]="notice_waterski",
  [NMK_SLCP]="notice_sailboat", [NMK_UPCP]="notice_rowboat", [NMK_SLBP]="notice_sailboard", [NMK_RADI]="notice_vhf", [NMK_WTBP]="notice_waterbike", [NMK_HSCP]="notice_speedboat", [NMK_LBGP]="notice_slipway" };

char **cluster_map(Obja_t obj) {
  switch (obj) {
    case NOTMRK:
      return notice_map;
    case SMCFAC:
      return scf_map;
    default:
      return NULL;
  }
}

char *lookupShape(char *shape, Obja_t obj) {
  Enum_t idx;
  if (((obj == TOPMAR) || (obj == DAYMAR)) && ((idx = enumValue(shape, TOPSHP)) != 0))
    return top_shapes[idx];
  return shape;
}

Item_t *findNext() {
  if (item != NULL) {
    for (item = item->next; item != NULL; item = item->next) {
      if (item->objs.obj == type)
        break;
    }
  }
  return item;
}

Item_t *findItem(char *obj) {
  type = enumType(obj);
  item = &map;
  return findNext();
}

double lon2x(double lon) {
  return ((lon - minlon) * 256.0 * 2048.0 / 180.0);
}

double lat2y(double lat) {
  return (((1.0 - log(tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * 256.0 * 4096.0) - top);
}

XY_t radial(XY_t centre, double radius, double angle) {
  XY_t position;
  position.x = centre.x - (radius * mile * sin(d2r(angle)));
  position.y = centre.y - (radius * mile * cos(d2r(angle)));
  return position;
}

void render(char *symbols) {
  top = (1.0 - log(tan(maxlat * M_PI/180.0) + 1.0 / cos(maxlat * M_PI/180.0)) / M_PI) / 2.0 * 256.0 * 4096.0;
  mile = lat2y(maxlat) - lat2y(maxlat + (1.0/60.0));

  printf("<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\"\n");
  printf(" width=\"%f\" height=\"%f\">\n", lon2x(maxlon), lat2y(minlat));
  
//  printf(" <rect x=\"0\" y=\"0\" width=\"%g\" height=\"%g\" style=\"fill:#b5d0d0;fill-opacity:1\"/>\n", lon2x(maxlon), lat2y(minlat)); //****** for testing only!!!!*******

  char line[2000];
  FILE *fp = fopen(symbols, "r");
  if (fp == NULL) {
    fprintf(stderr, "Symbol file not found\n");
    exit(EXIT_FAILURE);
  }
	while (fgets(line, 2000, fp) != NULL) {
    printf("%s", line);
		char *ele = strtok(line, " \t<");
    if (strcmp(ele, "symbol") == 0) {
      char *name = NULL;
      double width = 0.0;
      double height = 0.0;
      char *token = strtok(NULL, " =");
      do {
        char *val = strtok(NULL, "'\"");
        if (strcmp(token, "id") == 0) {
          name = strdup(val);
        } else if (strcmp(token, "width") == 0) {
          width = atof(val);
        } else if (strcmp(token, "height") == 0) {
          height = atof(val);
        }
        token = strtok(NULL, " =");
      } while (token != NULL);
      int hash;
      char *ptr;
      for (hash = 0, ptr = name; *ptr != 0; ptr++) {
        hash += *ptr;
      }
      hash %= 100;
      Hash_t *link = calloc(1, sizeof(Hash_t));
      link->next = hashtable[hash];
      link->name = name;
      link->width = width;
      link->height = height;
      hashtable[hash] = link;
    }
  }
  fclose(fp);
  main_rules(NULL, NULL);
  printf("</svg>\n");
}

Hash_t *lookupSymbol(char *symbol) {
  int hash;
  char *ptr;
  if (symbol == NULL) return NULL;
  for (hash = 0, ptr = symbol; *ptr != 0; ptr++) {
    hash += *ptr;
  }
  hash %= 100;
  Hash_t *link;
  for (link = hashtable[hash]; link != NULL; link = link->next) {
    if (strcmp(symbol, link->name) == 0)
      break;
  }
  return link;
}

XY_t findCentroid(Item_t *item) {
  XY_t coord;
  if (item->flag == WAY) {
    double sx = 0.0;
    double sy = 0.0;
    double sd = 0.0;
    double lx = 0.0;
    double ly = 0.0;
    if ((item->type.way.flink->ref != item->type.way.blink->ref) && (item->type.way.blink->ref->next != NULL)) {
      lx = lon2x(item->type.way.blink->ref->next->type.node.lon);
      ly = lat2y(item->type.way.blink->ref->next->type.node.lat);
    } else {
      lx = lon2x(item->type.way.blink->ref->type.node.lon);
      ly = lat2y(item->type.way.blink->ref->type.node.lat);
    }
    Ref_t *link = item->type.way.blink;
    while (link != NULL) {
      double x = lon2x(link->ref->type.node.lon);
      double y = lat2y(link->ref->type.node.lat);
      double d = sqrt(pow((x-lx), 2) + pow((y-ly), 2));
      sx += (x * d);
      sy += (y * d);
      sd += d;
      lx = x;
      ly = y;
      link = link->blink;
    }
    coord.x = sd > 0.0 ? sx/sd : 0.0;
    coord.y = sd > 0.0 ? sy/sd : 0.0;
  } else {
    coord.x = lon2x(item->type.node.lon);
    coord.y = lat2y(item->type.node.lat);
  }
  return coord;
}

int placeSymbol(XY_t coord, Obja_t obj, char *symbol, char * panel, char *colour, Handle_t handle, double dx, double dy, double angle) {
  if ((symbol == NULL) || (panel == NULL)) return 0;
  char sympan[100];
  strcpy(sympan, lookupShape(symbol, obj));
  strcat(sympan, panel);
  Hash_t *params = lookupSymbol(sympan);
  if (params != NULL) {
    double x, y;
    switch (handle) {
      case TL:
        x = -dx; y = -dy;
        break;
      case TC:
        x = -dx - params->width/2; y = -dy;
        break;
      case TR:
        x = -dx - params->width; y = -dy;
        break;
      case LC:
        x = -dx; y = -dy - params->height/2;
        break;
      case RC:
        x = -dx - params->width; y = -dy - params->height/2;
        break;
      case BL:
        x = -dx; y = -dy - params->height;
        break;
      case BC:
        x = -dx - params->width/2; y = -dy - params->height;
        break;
      case BR:
        x = -dx - params->width; y = -dy - params->height;
        break;
      default:
        x = -dx - params->width/2; y = -dy - params->height/2;
    }
    printf("<g id=\"%d\" transform=\"translate(%f, %f) scale(%f) translate(%f, %f) rotate(%f, %f, %f) \"> <use xlink:href=\"#%s\" style=\"fill:%s\"/> </g>\n",
           ++ref, coord.x, coord.y, symbolScale[zoom], x, y, angle, -dx-x, -dy-y, sympan, colour);
  }
  return ref;
}

int renderSymbol(Item_t *item, Obja_t obj, char *symbol, char * panel, char *colour, Handle_t handle, double dx, double dy, double angle) {
  return placeSymbol(findCentroid(item), obj, symbol, panel, colour, handle, dx, dy, angle);
}

int renderColourSymbol(Item_t *item, Obja_t obj, char *symbol, char * panel, char *colour, Handle_t handle, double dx, double dy, double angle) {
  return renderSymbol(item, obj, symbol, panel, body_colours[enumValue(colour, COLOUR)], handle, dx, dy, angle);
}

void renderCluster(Item_t *item, char *type) {
  int n = countObjects(item, type);
  Obja_t obja = enumType(type);
  Atta_t atta = enumAttribute("category", obja);
  char **map = cluster_map(obja);
  if (map == NULL) return;
  switch (n) {
    case 0: {
      Obj_t *obj = getObj(item, obja, 0);
      int n = countValues(getTag(obj, atta));
      switch (n) {
        case 1:
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 0)], "", "", CC, 0, 0, 0);
          break;
        case 2:
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 0)], "", "", RC, 0, 0, 0);
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 1)], "", "", LC, 0, 0, 0);
          break;
        case 3:
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 0)], "", "", BC, 0, 0, 0);
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 1)], "", "", TR, 0, 0, 0);
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 2)], "", "", TL, 0, 0, 0);
          break;
        case 4:
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 0)], "", "", BR, 0, 0, 0);
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 1)], "", "", BL, 0, 0, 0);
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 2)], "", "", TR, 0, 0, 0);
          renderSymbol(item, obja, map[getTagEnum(obj, atta, 3)], "", "", TL, 0, 0, 0);
          break;
      }
    }
      break;
    case 1:
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 1), atta, 0)], "", "", CC, 0, 0, 0);
      break;
    case 2:
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 1), atta, 0)], "", "", RC, 0, 0, 0);
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 2), atta, 0)], "", "", LC, 0, 0, 0);
      break;
    case 3:
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 1), atta, 0)], "", "", BC, 0, 0, 0);
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 2), atta, 0)], "", "", TR, 0, 0, 0);
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 3), atta, 0)], "", "", TL, 0, 0, 0);
      break;
    case 4:
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 1), atta, 0)], "", "", BR, 0, 0, 0);
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 2), atta, 0)], "", "", BL, 0, 0, 0);
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 3), atta, 0)], "", "", TR, 0, 0, 0);
      renderSymbol(item, obja, map[getTagEnum(getObj(item, obja, 4), atta, 0)], "", "", TL, 0, 0, 0);
      break;
  }
}

void renderNotice(Item_t *item) {
  double dx = 0.0, dy = 0.0;
  int n = countObjects(item, "notice");
  if (n > 2) {
    renderCluster(item, "notice");
    return;
  }
  switch (item->objs.obj) {
    case BCNCAR:
    case BCNISD:
    case BCNLAT:
    case BCNSAW:
    case BCNSPP:
    case BCNWTW:
      dy = 45.0;
      break;
    case NOTMRK:
      dy = 0.0;
      break;
    default:
      return;
  }
  bool swap = false;
  if (n == 2) {
    for (int i = 0; i <=2; i++) {
      Obj_t *obj = getObj(item, NOTMRK, i);
      if (obj == NULL) continue;
      Atta_t add;
      int idx = 0;
      while ((add = getTagEnum(obj, ADDMRK, idx++)) != MRK_UNKN) {
        if ((add == MRK_LTRI) && (i == 2)) swap = true;
        if ((add == MRK_RTRI) && (i != 2)) swap = true;
      }
    }
  }
  for (int i = 0; i <=2; i++) {
    Obj_t *obj = getObj(item, NOTMRK, i);
    if (obj == NULL) continue;
    Atta_t category = getTagEnum(obj, CATNMK, i);
    Atta_t add;
    int idx = 0;
    int top=0, bottom=0, left=0, right=0;
    while ((add = getTagEnum(obj, ADDMRK, idx++)) != MRK_UNKN) {
      switch (add) {
        case MRK_TOPB:
          top = add;
          break;
        case MRK_BOTB:
        case MRK_BTRI:
          bottom = add;
          break;
        case MRK_LTRI:
          left = add;
          break;
        case MRK_RTRI:
          right = add;
          break;
        default:
          break;
      }
    }
    double orient = getTag(obj, ORIENT) != NULL ? getTag(obj, ORIENT)->val.val.f : 0.0;
    double flip = 0.0;
    char *symb = notice_map[category];
    char *base = "";
    char *colour = "black";
    switch (category) {
      case NMK_NOVK...NMK_NWSH:
      case NMK_NMTC...NMK_NLBG:
        base = "notice_a";
        break;
      case NMK_MVTL...NMK_CHDR:
        base = "notice_b";
        break;
      case NMK_PRTL...NMK_PRTR:
      case NMK_OVHC...NMK_LBGP:
        base = "notice_e";
        colour = "white";
        break;
      default:
        break;
    }
    switch (category) {
      case NMK_MVTL:
      case NMK_ANKP:
      case NMK_PRTL:
      case NMK_MWAL:
      case NMK_MWAR:
        flip = 180.0;
        break;
      case NMK_SWWR:
      case NMK_WRSL:
      case NMK_WARL:
        flip = -90.0;
        break;
      case NMK_SWWC:
      case NMK_SWWL:
      case NMK_WLSR:
      case NMK_WALR:
        flip = 90.0;
        break;
      default:
        break;
    }
    if (n == 2) {
      dx = (((i != 2) && swap) || ((i == 2) && !swap)) ? -30.0 : 30.0;
    }
    if (top == MRK_TOPB)
      renderSymbol(item, NOTMRK, "notice_board", "", "", BC, dx, dy, orient);
    if (bottom == MRK_BOTB)
      renderSymbol(item, NOTMRK, "notice_board", "", "", BC, dx, dy, orient+180);
    if (bottom == MRK_BTRI)
      renderSymbol(item, NOTMRK, "notice_triangle", "", "", BC, dx, dy, orient+180);
    if (left == MRK_LTRI)
      renderSymbol(item, NOTMRK, "notice_triangle", "", "", BC, dx, dy, orient-90);
    if (right == MRK_RTRI)
      renderSymbol(item, NOTMRK, "notice_triangle", "", "", BC, dx, dy, orient+90);
    renderSymbol(item, NOTMRK, base, "", "", CC, dx, dy, orient);
    renderSymbol(item, NOTMRK, symb, "", colour, CC, dx, dy, orient+flip);
  }
}

void renderFlare(Item_t *item) {
  char *col = light_colours[COL_MAG];
  Obj_t *obj = getObj(item, LIGHTS, 0);
  Tag_t *tag;
  if (((tag = getTag(obj, COLOUR)) != NULL) && (tag->val.val.l->next == NULL)) {
    col = light_colours[tag->val.val.l->val];
  }
  renderSymbol(item, LIGHTS, "light", "", col, CC, 0, 0, 120);
}

void renderSector(Item_t *item, int s, char *text, char *style, double offset, int dy) {
  Obj_t *sector;
  double start, end;
  Tag_t *tag;
  XY_t p0, p1;
  double r0, r1;
  double b0, b1, span;
  char *col;
  XY_t pos = findCentroid(item);
  if ((sector = getObj(item, LIGHTS, s)) != NULL) {
    strcpy(string1, (tag = getTag(sector, LITRAD)) != NULL ? tag->val.val.a : "0.2");
    if (((tag = getTag(sector, CATLIT)) != NULL) && (testTag(tag, LIT_DIR)) && ((tag = getTag(sector, ORIENT)) != NULL)) {
      b0 = fmod(540.0 - tag->val.val.f, 360.0);
      if ((tag = getTag(sector, COLOUR)) != NULL) {
        col = light_colours[tag->val.val.l->val];
        r0 = atof(string1);
        p0 = radial(pos, r0, b0);
        printf("<path d=\"M %g,%g L %g,%g\" style=\"fill:none;stroke:#808080;stroke-width:%g;stroke-dasharray:%g\"/>\n",
               pos.x, pos.y, p0.x, p0.y, (4 * symbolScale[zoom]), (20 * symbolScale[zoom]));
        start = fmod(b0 + 2.0, 360.0);
        end = fmod(360.0 + b0 - 2.0, 360.0);
        Obj_t *adj;
        for (int i = s-1; i <= s+1; i++) {
          if (i == s) continue;
          if ((adj = getObj(item, LIGHTS, i)) == NULL) continue;
          Tag_t *tag;
          if (((tag = getTag(adj, CATLIT)) != NULL) && (testTag(tag, LIT_DIR)) && ((tag = getTag(adj, ORIENT)) != NULL)) {
            b1 = fmod(540.0 - tag->val.val.f, 360.0);
            if (fabs(b0 - b1) > 180.0) {
              if (b0 < b1) b0 += 360.0;
              else b1 += 360.0;
            }
            if (fabs(b0 - b1) < 4.0) {
              if (b1 > b0) start = fmod((720.0 + b0 + b1) / 2.0, 360.0);
              else end = fmod((720.0 + b0 + b1) / 2.0, 360.0);
            }
          }
        }
        p0 = radial(pos, r0, start);
        p1 = radial(pos, r0, end);
        printf("<path id=\"%d\" d=\"M %g,%g A %g,%g,0,0,1,%g,%g\" style=\"fill:none;stroke:%s;stroke-width:%g\"/>\n",
               ++ref, p0.x, p0.y, r0*mile, r0*mile, p1.x, p1.y, col, (20 * symbolScale[zoom]));
        if (tag->val.val.l->next != NULL) {
          char *col = light_colours[tag->val.val.l->next->val];
          r1 = r0 - (20 * symbolScale[zoom]/mile);
          p0 = radial(pos, r1, start);
          p1 = radial(pos, r1, end);
          printf("<path d=\"M %g,%g A %g,%g,0,0,1,%g,%g\" style=\"fill:none;stroke:%s;stroke-width:%g\"/>\n",
                 p0.x, p0.y, r1*mile, r1*mile, p1.x, p1.y, col, (20 * symbolScale[zoom]));
        }
      }
    } else if ((tag = getTag(sector, SECTR1)) != NULL) {
      start = fmod(540.0 - tag->val.val.f, 360.0);
      if ((tag = getTag(sector, SECTR2)) != NULL) {
        end = fmod(540.0 - tag->val.val.f, 360.0);
        start += start < end ? 360.0 : 0.0;
        if ((tag = getTag(sector, COLOUR)) != NULL) {
          char *ttok, *etok;
          char *radstr = strdup(string1);
          int arc = 0;
          col = light_colours[tag->val.val.l->val];
          r0 = 0.0;
          b0 = b1 = start;
          for (char *tpl = strtok_r(radstr, ";", &ttok); tpl != NULL; tpl = strtok_r(NULL, ";", &ttok)) {
            p0 = radial(pos, r0, b0);
            span = 0.0;
            char *ele = strtok_r(tpl, ":", &etok);
            if ((*tpl == ':') && (r0 == 0.0)) {
              r1 = 0.2;
            } else if (*tpl != ':') {
              r1 = atof(tpl);
              ele = strtok_r(NULL, ":", &etok);
            }
            while (ele != NULL) {
              if (isalpha(*ele)) {
                if (strcmp(ele, "suppress") == 0) arc = 2;
                else if (strcmp(ele, "dashed") == 0) arc = 1;
                else arc = 0;
              } else {
                span = atof(ele);
              }
              ele = strtok_r(NULL, ":", &etok);
            }
            if (span == 0.0) {
              char *back = (ttok != NULL) ? strstr(ttok, "-") : NULL;
              if (back != NULL) {
                span = b0 - end + atof(back);
              } else {
                span = b0 - end;
              }
            }
            if (r1 != r0) {
              p1 = radial(pos, r1, b0);
              if (!((start == 180.0) && (end == 180.0)))
                printf("<path d=\"M %g,%g L %g,%g\" style=\"fill:none;stroke:#808080;stroke-width:%g;stroke-dasharray:%g\"/>\n",
                       p0.x, p0.y, p1.x, p1.y, (4 * symbolScale[zoom]), (20 * symbolScale[zoom]));
              r0 = r1;
              p0 = p1;
            }
            if (span < 0.0) {
              b1 = end - span;
              b1 = b1 > b0 ? b0 : b1;
              b0 = b1;
              b1 = end;
              p0 = radial(pos, r0, b0);
            } else {
              b1 = b0 - span;
              b1 = b1 < end ? end : b1;
            }
            p1 = radial(pos, r1, b1);
            if ((b0 == 180.0) && (b1 == 180.0)) {
              span = 360.0;
              p1 = radial(pos, r1, b1+0.01);
            }
            if (arc == 0) {
              if (p0.x < p1.x)
                printf("<path id=\"%d\" d=\"M %g,%g A %g,%g,0,%d,1,%g,%g\" style=\"fill:none;stroke:%s;stroke-width:%g\"/>\n",
                       ++ref, p0.x, p0.y, r1*mile, r1*mile, span>180.0, p1.x, p1.y, col, (20 * symbolScale[zoom]));
              else
                printf("<path id=\"%d\" d=\"M %g,%g A %g,%g,0,%d,0,%g,%g\" style=\"fill:none;stroke:%s;stroke-width:%g\"/>\n",
                       ++ref, p1.x, p1.y, r1*mile, r1*mile, span>180.0, p0.x, p0.y, col, (20 * symbolScale[zoom]));
              if (text != NULL) {
                double chord = sqrt(pow((p0.x - p1.x), 2) + pow((p0.y - p1.y), 2));
                if ((chord > (strlen(text) * textScale[zoom] * 50)) || ((b0 == 180.0) && (b1 == 180.0)))
                  drawLineText(item, text, style, offset, dy, ref);
              }
            } else if (arc == 1) {
              printf("<path d=\"M %g,%g A %g,%g,0,%d,1,%g,%g\" style=\"fill:none;stroke:%s;stroke-width:%g;stroke-opacity:0.5;stroke-dasharray:%g\"/>\n",
                     p0.x, p0.y, r1*mile, r1*mile, span>180.0, p1.x, p1.y, col, (10 * symbolScale[zoom]), (30 * symbolScale[zoom]));
            }
            if ((arc == 0) && (tag->val.val.l->next != NULL)) {
              char *col = light_colours[tag->val.val.l->next->val];
              double r2 = r1 - (20 * symbolScale[zoom]/mile);
              XY_t p2 = radial(pos, r2, b0);
              XY_t p3 = radial(pos, r2, b1);
              printf("<path d=\"M %g,%g A %g,%g,0,%d,1,%g,%g\" style=\"fill:none;stroke:%s;stroke-width:%g\"/>\n",
                     p2.x, p2.y, r1*mile, r1*mile, span>180.0, p3.x, p3.y, col, (20 * symbolScale[zoom]));
            }
            b0 = b1;
            if (b0 == end) break;
          }
          if (!((start == 180.0) && (end == 180.0)))
            printf("<path d=\"M %g,%g L %g,%g\" style=\"fill:none;stroke:#808080;stroke-width:%g;stroke-dasharray:%g\"/>\n",
                   pos.x, pos.y, p1.x, p1.y, (4 * symbolScale[zoom]), (20 * symbolScale[zoom]));
          free(radstr);
        }
      }
    }
  }
}

bool compareTypes(Item_t *item, char *types) {
  strcpy(string1, types);
  char *typstr = strtok(string1, "|");
  bool result = false;
  while (typstr != NULL) {
    if (enumType(typstr) == item->objs.obj) {
      result = true;
      break;
    }
    typstr = strtok(NULL, "|");
  }
  return result;
}

bool compareObjects(Item_t *item, char *objects) {
  strcpy(string1, objects);
  char *objstr = strtok(string1, "|");
  bool result = false;
  while (objstr != NULL) {
    Obja_t objl = enumType(objstr);
    Obj_t *obj = item->objs.next;
    while (obj != NULL) {
      if (obj->obj == objl) {
        result = true;
        break;
      }
      obj = obj->next;
    }
    if (result) break;
    objstr = strtok(NULL, "|");
  }
  return result;
}

int countObjects(Item_t *item, char *object) {
  int result = 0;
  Obja_t obj = enumType(object);
  Obj_t *link = item->objs.next;
  while (link != NULL) {
    if ((link->obj == obj) && (link->idx > result))
      result = link->idx;
    link = link->next;
  }
  return result;
}

bool compareAttributes(Obj_t *objl, char *attributes) {
  if (objl == NULL) return false;
  strcpy(string1, attributes);
  char *attstr = strtok(string1, "|");
  bool result = false;
  while (attstr != NULL) {
    Atta_t att = enumAttribute(attstr, objl->obj);
    if (att == 0) break;
    Tag_t *tag = objl->tags;
    while (tag != NULL) {
      if (tag->val.key == att) {
        result = true;
        break;
      }
      tag = tag->next;
    }
    if (result) break;
    attstr = strtok(NULL, "|");
  }
  return result;
}

int countValues(Tag_t *tag) {
  int i = 0;
  Lst_t *ptr;
  if (tag->val.type == E)
    return 1;
  if (tag->val.type == L)
    for(i = 0, ptr = tag->val.val.l; ptr != NULL; ptr = ptr->next, i++);
  return i;
}

bool compareLiterals(char *values, char *cases) {
  char *valsav;
  char *cmpsav;
  if (values == NULL) return false;
  strcpy(string1, cases);
  char *cmpstr = strtok_r(string1, "|", &cmpsav);
  strcpy(string2, values);
  bool result = false;
  while (cmpstr != NULL) {
    char *valstr = strtok_r(string2, ";", &valsav);
    while (valstr != NULL) {
      if (strcmp(valstr, cmpstr) == 0) {
        result = true;
        break;
      }
      valstr = strtok_r(NULL, ";", &valsav);
    }
    if (result) break;
    strcpy(string2, values);
    cmpstr = strtok_r(NULL, "|", &cmpsav);
  }
  return result;
}

bool compareValues(Tag_t *val, char *cases) {
  if (val == NULL) return false;
  return compareLiterals(stringValue((val->val)), cases);
}

double scaleStyle(char *style) {
  double size = 0.0;
  strcpy(string1, style);
  printf("style=\"");
  char *ele = strtok(string1, " :");
  do {
    char *val = strtok(NULL, " ,;");
    if (strcmp(ele, "stroke-width") == 0) {
      printf("stroke-width:%f; ", atof(val) * symbolScale[zoom]);
    }
    else if (strcmp(ele, "stroke-dasharray") == 0) {
      printf("stroke-dasharray:%f,", atof(val) * symbolScale[zoom]);
      val = strtok(NULL, " ,;");
      printf("%f; ", atof(val) * symbolScale[zoom]);
    }
    else if (strcmp(ele, "font-size") == 0) {
      size = atof(val) * textScale[zoom];
      printf("font-size:%f; ", size);
    }
    else printf("%s:%s; ", ele, val);
    ele = strtok(NULL, " :");
  } while (ele != NULL);
  printf("\" ");
  return size;
}

int drawVector(Item_t *item, char *style, int dir) {
  if (item->flag == WAY) {
    Ref_t *link = dir < 0 ? item->type.way.flink : item->type.way.blink;
    if (link != NULL) {
      printf("<path id=\"%d\" d=\"M ", ++ref);
      while (link != NULL) {
        printf("%f %f ", lon2x(link->ref->type.node.lon), lat2y(link->ref->type.node.lat));
        link = dir < 0 ?  link->flink : link->blink;
      }
      if (dir == 0) printf("Z");
      printf("\" ");
      scaleStyle(style);
      printf("/>\n");
      return ref;
    }
  }
  return 0;
}

int drawLine(Item_t *item, char *style) {
  if (item->flag != WAY) return 0;
  Ref_t *link = item->type.way.blink;
  double sx = 0.0;
  double lx = 0.0;
  double ly = 0.0;
  while (link != NULL) {
    double x = lon2x(link->ref->type.node.lon);
    double y = lat2y(link->ref->type.node.lat);
    if ((lx == 0.0) && (ly == 0.0)) {
      lx = x;
      ly = y;
    }
    double d = sqrt(pow((x-lx), 2) + pow((y-ly), 2));
    sx += ((x - lx) * d);
    lx = x;
    ly = y;
    link = link->blink;
  }
  return drawVector(item, style, sx < 0.0 ? -1 : +1);
}

void drawLineArrows(Item_t *item) {
  if (item->flag != WAY) return;
  double size = 200.0 * symbolScale[zoom];
  Ref_t *link = item->type.way.blink;
  if (link == NULL) return;
  XY_t last = {0,0};
  XY_t next = {lon2x(link->ref->type.node.lon), lat2y(link->ref->type.node.lat)};
  XY_t old = next;
  XY_t new = {0,0};
  bool gap = false;
  bool piv = false;
  double angle = 0.0;
  double rem = 0.0;
  double len = size;
  while (true) {
    if (rem > len) {
      if (piv) {
        new.x = last.x + (len * cos(angle));
        new.y = last.y + (len * sin(angle));
        piv = false;
      } else {
        new.x = old.x + (len * cos(angle));
        new.y = old.y + (len * sin(angle));
      }
      if (!gap) {
        placeSymbol(old, TSSLPT, "lane_arrow", "", "", BC, 0, 0, r2d(atan2((new.y - old.y), (new.x - old.x)))+90);
      }
      gap = !gap;
      old = new;
      len = gap ? (size / 2.0): size;
      rem = sqrt(pow((next.x-new.x), 2) + pow((next.y-new.y), 2));
    } else {
      len -=rem;
      last = next;
      if ((link = link->blink) == NULL) return;
      next.x = lon2x(link->ref->type.node.lon); next.y = lat2y(link->ref->type.node.lat);
      rem = sqrt(pow((next.x-last.x), 2) + pow((next.y-last.y), 2));
      angle = atan2((next.y - last.y), (next.x - last.x));
      piv = true;
    }
  }
}

int drawArea(Item_t *item, char *style) {
  return drawVector(item, style, 0);
}

int drawLineText(Item_t *item, char *text, char *style, double offset, int dy, int path) {
  printf("<text id=\"%d\" ", ++ref);
  scaleStyle(style);
  if (offset > 1.0)
    printf("><textPath xlink:href=\"#%d\" startOffset=\"%f\"><tspan dy=\"%f\">%s</tspan></textPath></text>\n", path, offset*text[zoom], dy*symbolScale[zoom], text);
  else
    printf("><textPath xlink:href=\"#%d\" startOffset=\"%f%%\"><tspan dy=\"%f\">%s</tspan></textPath></text>\n", path, offset*100, dy*symbolScale[zoom], text);
  return ref;
}

int drawAreaText(Item_t *item, char *text, char *style) {
  if (item->flag == WAY) {
    XY_t coord = findCentroid(item);
    if ((coord.x > 0.0) || (coord.y > 0.0)) {
      printf("<text id=\"%d\" ", ++ref);
      scaleStyle(style);
      printf("x=\"%f\" y=\"%f\">%s</text>\n", coord.x, coord.y, text);
    }
    return ref;
  }
  return 0;
}

int drawText(Item_t *item, char *text, char *style, double dx, double dy) {
  XY_t coord = findCentroid(item);
  if ((coord.x > 0.0) || (coord.y > 0.0)) {
    printf("<text id=\"%d\" ", ++ref);
    double size = scaleStyle(style) * 1.1;
    double x = coord.x + (dx*symbolScale[zoom]);
    double y = coord.y + (dy*symbolScale[zoom]);
    char *line = strtok(text, "\n");
    double dy = 0.0;
    printf("y=\"%f\">", y);
    while (line != NULL) {
      printf("<tspan x=\"%f\" dy=\"%f\">%s</tspan>\n", x, dy, line);
      dy = size;
      line = strtok(NULL, "\n");
    }
    printf("</text>\n");
    return ref;
  }
  return 0;
}


char *charString(Item_t *item, char *type, int idx) {
  strcpy(string1, "");
  Tag_t *tag = NULL;
  Obj_t *obj = getObj(item, enumType(type), idx);
  switch (enumType(type)) {
    case CGUSTA:
      strcpy(string1, "CG");
      if ((obj != NULL) && (tag = getTag(obj, COMCHA)) != NULL)
        sprintf(strchr(string1, 0), " Ch.%s", stringValue(tag->val));
      break;
    case FOGSIG:
      if (obj != NULL) {
        if ((tag = getTag(obj, CATFOG)) != NULL)
          strcat(string1, fog_signals[tag->val.val.e]);
        if ((tag = getTag(obj, SIGGRP)) != NULL)
          sprintf(strchr(string1, 0), "(%s)", stringValue(tag->val));
        else 
          strcat(string1, " ");
        if ((tag = getTag(obj, SIGPER)) != NULL)
          sprintf(strchr(string1, 0), "%ss ", stringValue(tag->val));
        if ((tag = getTag(obj, VALMXR)) != NULL)
          sprintf(strchr(string1, 0), "%sM", stringValue(tag->val));
      }
      break;
    case RTPBCN:
      if (obj != NULL) {
        if ((tag = getTag(obj, CATRTB)) != NULL)
          strcat(string1, rtb_map[tag->val.val.e]);
        if ((tag = getTag(obj, SIGGRP)) != NULL)
          sprintf(strchr(string1, 0), "(%s)", stringValue(tag->val));
        else 
          strcat(string1, " ");
        if ((tag = getTag(obj, SIGPER)) != NULL)
          sprintf(strchr(string1, 0), "%ss ", stringValue(tag->val));
        if ((tag = getTag(obj, VALMXR)) != NULL)
          sprintf(strchr(string1, 0), "%sM", stringValue(tag->val));
      }
      break;
    case SISTAT:
      strcpy(string1, "SS");
      if (obj != NULL) {
        if ((tag = getTag(obj, CATSIT)) != NULL)
          strcat(string1, sit_map[tag->val.val.l->val]);
        if ((tag = getTag(obj, COMCHA)) != NULL)
          sprintf(strchr(string1, 0), "\nCh.%s", stringValue(tag->val));
      }
      break;
    case SISTAW:
      strcpy(string1, "SS");
      if (obj != NULL) {
        if ((tag = getTag(obj, CATSIW)) != NULL)
          strcat(string1, siw_map[tag->val.val.l->val]);
        if ((tag = getTag(obj, COMCHA)) != NULL)
          sprintf(strchr(string1, 0), "\nCh.%s", stringValue(tag->val));
      }
      break;
    case LIGHTS:
      {
        int secmax = countObjects(item, "light");
        if ((idx == 0) && (secmax > 0)) {
          struct SECT {
            struct SECT *next;
            int dir;
            LitCHR_t chr;
            ColCOL_t col;
            ColCOL_t alt;
            char *grp;
            double per;
            double rng;
          } *lights = NULL;
          for (int i = secmax; i > 0; i--) {
            struct SECT *tmp = calloc(1, sizeof(struct SECT));
            tmp->next = lights;
            lights = tmp;
            obj = getObj(item, LIGHTS, i);
            if ((tag = getTag(obj, CATLIT)) != NULL) {
              lights->dir = testTag(tag, LIT_DIR);
            }
            if ((tag = getTag(obj, LITCHR)) != NULL) {
              lights->chr = tag->val.val.e;
              switch (lights->chr) {
                case CHR_AL:
                  lights->chr = CHR_F;
                  break;
                case CHR_ALOC:
                  lights->chr = CHR_OC;
                  break;
                case CHR_ALLFL:
                  lights->chr = CHR_LFL;
                  break;
                case CHR_ALFL:
                  lights->chr = CHR_FL;
                  break;
                case CHR_ALFFL:
                  lights->chr = CHR_FFL;
                  break;
                default:
                  break;
              }
            }
            if ((tag = getTag(obj, SIGGRP)) != NULL) {
              lights->grp = tag->val.val.a;
            } else {
              lights->grp = "";
            }
            if ((tag = getTag(obj, SIGPER)) != NULL) {
              lights->per = tag->val.val.f;
            }
            if ((tag = getTag(obj, VALNMR)) != NULL) {
              lights->rng = tag->val.val.f;
            }
            if ((tag = getTag(obj, COLOUR)) != NULL) {
              lights->col = tag->val.val.l->val;
              if (tag->val.val.l->next != NULL)
                lights->alt = tag->val.val.l->next->val;
            }
          }
          struct COLRNG {
            int col;
            double rng;
          } colrng[14];
          while (lights != NULL) {
            strcpy(string2, "");
            bzero(colrng, 14*sizeof(struct COLRNG));
            colrng[lights->col].col = 1;
            colrng[lights->col].rng = lights->rng;
            struct SECT *this = lights;
            struct SECT *next = lights->next;
            while (next != NULL) {
              if ((this->dir == next->dir) && (this->chr == next->chr) &&
                  (strcmp(this->grp, next->grp) == 0) && (this->per == next->per)) {
                colrng[next->col].col = 1;
                if (next->rng > colrng[next->col].rng)
                  colrng[next->col].rng = next->rng;
                struct SECT *tmp = lights;
                while (tmp->next != next) tmp = tmp->next;
                tmp->next = next->next;
                free(next);
                next = tmp->next;
              } else {
                next = next->next;
              }
            }
            if (this->chr != CHR_UNKN) {
              if (this->dir) strcpy(string2, "Dir.");
              strcat(string2, light_characters[this->chr]);
              if (strcmp(this->grp, "") != 0) {
                sprintf(strchr(string2, 0), "(%s)", this->grp);
              } else {
                if (strlen(string2) > 0) strcat(string2, ".");
              }
              int n = 0;
              for (int i = 0; i < 14; i++) if (colrng[i].col) n++;
              double max = 0.0;
              for (int i = 0; i < 14; i++) if (colrng[i].col && (colrng[i].rng > max)) max = colrng[i].rng;
              double min = max;
              for (int i = 0; i < 14; i++) if (colrng[i].col && (colrng[i].rng > 0.0) && (colrng[i].rng < min)) min = colrng[i].rng;
              if (min == max) {
                for (int i = 0; i < 14; i++) if (colrng[i].col) strcat(string2, light_letters[i]);
              } else {
                for (int i = 0; i < 14; i++) if (colrng[i].col && (colrng[i].rng == max)) strcat(string2, light_letters[i]);
                for (int i = 0; i < 14; i++) if (colrng[i].col && (colrng[i].rng < max) && (colrng[i].rng > min)) strcat(string2, light_letters[i]);
                for (int i = 0; i < 14; i++) if (colrng[i].col && colrng[i].rng == min) strcat(string2, light_letters[i]);
              }
              strcat(string2, ".");
              if (this->per > 0.0) sprintf(strchr(string2, 0), "%gs", this->per);
              if (max > 0.0) {
                sprintf(strchr(string2, 0), "%g", max);
                if (min != max) {
                  if (n == 2) strcat(string2, "/");
                  else if (n > 2) strcat(string2, "-");
                  if (min < max) sprintf(strchr(string2, 0), "%g", min);
                }
                strcat(string2, "M");
              }
              if (strlen(string1) > 0) strcat(string1, "\n");
              strcat(string1, string2);
            }
            lights = this->next;
            free(this);
            this = lights;
          }
        } else {
          if ((tag = getTag(obj, CATLIT)) != NULL) {
            if (testTag(tag, LIT_DIR))
              strcat(string1, "Dir");
          }
          if ((tag = getTag(obj, MLTYLT)) != NULL)
            sprintf(strchr(string1, 0), "%s", stringValue(tag->val));
          if ((tag = getTag(obj, LITCHR)) != NULL) {
            char *chrstr = strdup(stringValue(tag->val));
            Tag_t *grp = getTag(obj, SIGGRP);
            if (grp != NULL) {
              char *grpstr = strdup(stringValue(grp->val));
              switch (tag->val.val.e) {
                case CHR_QLFL:
                  sprintf(strchr(string1, 0), "Q(%s)+LFl", grpstr);
                  break;
                case CHR_VQLFL:
                  sprintf(strchr(string1, 0), "VQ(%s)+LFl", grpstr);
                  break;
                case CHR_UQLFL:
                  sprintf(strchr(string1, 0), "UQ(%s)+LFl", grpstr);
                  break;
                default:
                  sprintf(strchr(string1, 0), "%s(%s)", chrstr, grpstr);
                  break;
              }
              free(grpstr);
            } else {
              sprintf(strchr(string1, 0), "%s", chrstr);
            }
            free(chrstr);
          }
          if ((tag = getTag(obj, COLOUR)) != NULL) {
            int n = countValues(tag);
            if (!((n == 1) && (idx == 0) && (testTag(tag, COL_WHT)))) {
              if ((strlen(string1) > 0) && ((string1[strlen(string1)-1] != ')'))) 
                strcat(string1, ".");
              Lst_t *lst = tag->val.val.l;
              while (lst != NULL) {
                strcat(string1, light_letters[lst->val]);
                lst = lst->next;
              }
            }
          }
          if ((idx == 0) && (tag = getTag(obj, CATLIT)) != NULL) {
            if (testTag(tag, LIT_VERT))
              strcat(string1, "(vert)");
            if (testTag(tag, LIT_HORI))
              strcat(string1, "(hor)");
          }
          if ((strlen(string1) > 0) &&
              ((getTag(obj, SIGPER) != NULL) ||
               (getTag(obj, HEIGHT) != NULL) ||
               (getTag(obj, VALMXR) != NULL)) &&
              (string1[strlen(string1)-1] != ')'))
            strcat(string1, ".");
          if ((tag = getTag(obj, SIGPER)) != NULL)
            sprintf(strchr(string1, 0), "%ss", stringValue(tag->val));
          if ((idx == 0) && (item->objs.obj != LITMIN)) {
            if ((tag = getTag(obj, HEIGHT)) != NULL)
              sprintf(strchr(string1, 0), "%sm", stringValue(tag->val));
            if ((tag = getTag(obj, VALNMR)) != NULL)
              sprintf(strchr(string1, 0), "%sM", stringValue(tag->val));
          }
          if ((idx == 0) && (tag = getTag(obj, CATLIT)) != NULL) {
            if (testTag(tag, LIT_FRNT))
              strcat(string1, "(Front)");
            if (testTag(tag, LIT_REAR))
              strcat(string1, "(Rear)");
            if (testTag(tag, LIT_UPPR))
              strcat(string1, "(Upper)");
            if (testTag(tag, LIT_LOWR))
              strcat(string1, "(Lower)");
          }
        }
      }
      break;
    default: break;
 }
  return string1;
}