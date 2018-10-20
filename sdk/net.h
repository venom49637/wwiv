/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2017, WWIV Software Services             */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/*                                                                        */
/**************************************************************************/
#ifndef __INCLUDED_NET_H__
#define __INCLUDED_NET_H__

#ifdef __MSDOS__
#include "sdk/msdos_stdint.h"
#else
#include <cstdint>
#include <string>
#include <vector>
#endif // __MSDOS__

#ifndef DATEN_T_DEFINED
typedef uint32_t daten_t;
#define DATEN_T_DEFINED
#endif

#pragma pack(push, 1)

/* All network nodes except the destination will only look at:
 *   tosys - the destination system.  Is 0 if more than one system
 *   list_len - if (tosys==0) then list_len is the number of destination
 *     systems in a list immediately after the header.  NOTE this list will
 *     be 2*list_len bytes long, since each system entry is 2 bytes.
 *   length - is the overall length of the message, not including the header
 *     or the system ist length.  Thus the overall length of a message in
 *     the net will be (sizeof(net_header_rec) + 2*list_len + length)
 */
struct net_header_rec {
  uint16_t tosys,  /* destination system */
      touser,      /* destination user */
      fromsys,     /* originating system */
      fromuser,    /* originating user */
      main_type,   /* main message type */
      minor_type,  /* minor message type */
      list_len;    /* # of entries in system list */
  daten_t daten;   /* date/time sent */
  uint32_t length; /* # of bytes of msg after header */
  uint16_t method; /* method of compression */
};

/*
 * Please note that not all of these are used yet, some will probably never
 * be used, but sounded like a good idea at the time.
 */

#define main_type_net_info 0x0001      /* type 1 normal network updates */
#define main_type_email 0x0002         /* type 2 email by user number */
#define main_type_post 0x0003          /* type 3 post from sub host */
#define main_type_file 0x0004          /* type 4 file transfer system */
#define main_type_pre_post 0x0005      /* type 5 post to sub host */
#define main_type_external 0x0006      /* type 6 external message */
#define main_type_email_name 0x0007    /* type 7 email by user name */
#define main_type_net_edit 0x0008      /* type 8 network editor packet */
#define main_type_sub_list 0x0009      /* type 9 subs.lst update */
#define main_type_extra_data 0x000a    /* type 10 unused */
#define main_type_group_bbslist 0x000b /* type 11 network update from GC */
#define main_type_group_connect 0x000c /* type 12 network update from GC */
#define main_type_group_binkp 0x000d   /* type 13 network update from GC */
#define main_type_group_info 0x000e    /* type 14 misc update from GC */
#define main_type_ssm 0x000f           /* type 15 xxx read your mail */
#define main_type_sub_add_req 0x0010   /* type 16 add me to your sub */
#define main_type_sub_drop_req 0x0011  /* type 17 remove me from your sub*/
#define main_type_sub_add_resp 0x0012  /* type 18 status of add, 0=ok */
#define main_type_sub_drop_resp 0x0013 /* type 19 status of drop, 0=ok */
#define main_type_sub_list_info 0x0014 /* type 20 info for subs.lst file */

#define main_type_new_post 0x001a     /* type 26 post by sub name */
#define main_type_new_external 0x001b /* type 27 auto-proc ext. msgs */
#define main_type_game_pack 0x001c    /* type 28 game packs */

// Minor types used by main_type_net_info

#define net_info_general_message 0x0000
#define net_info_bbslist 0x0001
#define net_info_connect 0x0002
#define net_info_sub_lst 0x0003
#define net_info_wwivnews 0x0004
#define net_info_fbackhdr 0x0005
#define net_info_more_wwivnews 0x0006
#define net_info_categ_net 0x0007
#define net_info_network_lst 0x0008
#define net_info_file 0x0009
#define net_info_binkp 0x0010

/* these are in main_type_sub_*_resp, as the first byte of the text */
#define sub_adddrop_ok 0x00            /* you've been added/removed */
#define sub_adddrop_not_host 0x01      /* don't host that sub */
#define sub_adddrop_not_there 0x02     /* can't drop, you're not listed */
#define sub_adddrop_not_allowed 0x03   /* not allowed to add/drop you */
#define sub_adddrop_already_there 0x04 /* already in sub */
#define sub_adddrop_error 0xff         /* internal error */

struct net_contact_rec {
  uint16_t systemnumber,   /* System number of the contact */
      numcontacts,         /* # of contacts with system */
      numfails;            /* # of consec failed calls out */
  uint32_t firstcontact,   /* time of first contact w/ system */
      lastcontact,         /* time of most recent contact */
      lastcontactsent,     /* time of last contact w/data sent */
      lasttry;             /* time of last try to connect */
  uint32_t bytes_received, /* bytes received from system */
      bytes_sent,          /* bytes sent to system */
      bytes_waiting;       /* bytes waiting to be sent */
};

/* Each system will hold a file of these records.  Each record will hold the
 * data pertaining to all contacts with other systems.
 *
 * systemnumber tells what other system this data is for.
 * numcontacts - how many times a connection has been made with that system
 * numfails tells how many times this system has tried to call this other
 *   system but failed.  This is consecutive times, and is zeroed out
 *   when a successful connection is made by calling out.
 * firstcontact is time in unix format of the last time that system has been
 *   contacted on the net.
 * lastcontact is the time the last contact with that system was made.
 * lastcontactsent is the last time data was SENT to that system.
 * lasttry is the last time we tried to call out to that system.
 * bytes_received is number of bytes received from that system.
 * bytes_sent is number of bytes sent to that system.
 * bytes_waiting is the number of bytes waiting to be sent out to that
 *   system.
 *
 * This data is maintained so that the BBS can decide which other system to
 * attempt connection with next.
 */
struct net_system_list_rec {
  // system number of the system
  uint16_t sysnum;
  /* phone number of system */
  char phone[13];
  /* name of system */
  char name[40];
  /* group of the system */
  uint8_t group;
  /* max baud rate of system */
  uint16_t speed;
  /* other info about sys (bit-mapped) */
  uint16_t other;
  /* how to get there */
  uint16_t forsys;
  /* how long to get there */
  int16_t numhops;
  union {
    /* routing factor */
    uint16_t rout_fact;
    /* cost factor */
    float cost;
    /* temporary variable */
    int32_t temp;
  } xx;
};

/**
 * Contains the metadata for each network.
 *
 * On disk format for networks.dat
 */
struct net_networks_rec_disk {
  /* type of network */
  uint8_t type;
  /* network name */
  char name[16];
  /* directory for net data */
  char dir[69];
  /* system number */
  uint16_t sysnum;
  uint8_t padding[12];
};

#pragma pack(pop)

// This data is all read in from a text file which holds info about all of
// the systems in the network.  This text file doesn't hold connection info
// between the systems.  The purpose of all records should be obvious.

// BBSLIST designators

#define other_inet 0x0001         /* $ - System is PPP capable         */
#define other_fido 0x0002         /* \ - System run Fido frontend      */
#define other_telnet 0x0004       /* | - System is a telnet node       */
#define other_no_links 0x0008     /* < - System refuses links          */
#define other_fts_blt 0x0010      /* > - System uses FTS/BLT system    */
#define other_direct 0x0020       /* ! - System accepts direct connects*/
#define other_unregistered 0x0040 /* / - System is unregistered        */
#define other_fax 0x0080          /* ? - System accepts faxes          */
#define other_end_system 0x0100   /* _ - System is a dead end node     */
#define other_net_server 0x0200   /* + - System is network server      */
#define other_unused 0x0400       /* = - Unused identifier 2           */

/* system type designators */
#define other_net_coord 0x0800   /* & - NC */
#define other_group_coord 0x1000 /* % - GC */
#define other_area_coord 0x2000  /* ^ - AC */
#define other_subs_coord 0x4000  /* ~ - Sub Coordinator */

/*
 * This data is also read in from a text file.  It tells how much it costs for
 * sysnum to call out to other systems.  It is stored in connect.net.
 * This is never written as binary data.
 */
struct net_interconnect_rec {
  /* outward calling system */
  uint16_t sysnum;
  /* num systems it can call */
  uint16_t numsys;

#ifndef __MSDOS__
  // This requires modern compilers
  net_interconnect_rec() : sysnum(0), numsys(0) {}
  void clear() noexcept {
    numsys = 0;
    sysnum = 0;
    connect.clear();
    cost.clear();
  }
  /* points to an array of numsys integers that tell which
   * other systems sysnum can connect to
   */
  std::vector<uint16_t> connect;
  /*
   * cost[] - points to an array of numsys floating point numbers telling
   *   how much it costs to connect to that system, per minute.  ie, it would
   *   cost (cost[1]) dollars per minute for sysnum to call out to system
   *   number (connect[1]).
   */
  std::vector<float> cost;
#endif // __MSDOS__
};

#ifndef __MSDOS__

// This data is not serialized to disk, but parsed
// on demand from callout.net
/**
 * Contains per-node data in callout.net
 */
struct net_call_out_rec {
  net_call_out_rec() = default;
  net_call_out_rec(const std::string& f, uint16_t sn, uint16_t mn, uint16_t op, uint16_t ca,
                   int8_t nh, int8_t xh, const std::string& pw, uint8_t tpd, uint16_t nk)
      : ftn_address(f), sysnum(sn), macnum(mn), options(op), call_every_x_minutes(ca), min_hr(nh),
        max_hr(xh), session_password(pw), min_k(nk) {}
  // FTN Address.
  std::string ftn_address;
  /* system number */
  uint16_t sysnum = 0;
  /* macro/script to use */
  uint16_t macnum = 0;
  /* bit mapped */
  uint16_t options = 0;
  /* hours between callouts */
  uint16_t call_every_x_minutes = 0;
  /* callout min hour */
  int8_t min_hr = -1;
  /* callout max hour */
  int8_t max_hr = -1;
  /* password for system */
  std::string session_password;
  /* minimum # k before callout */
  uint16_t min_k = 0;
};
#endif // __MSDOS__

/* This record holds info about other systems that the sysop has determined
 * that we can call out to.
 *
 * sysnum - system number that we can call
 * macnum - macro/script number to use to call that system.  This is set
 *   to zero if we just dial it directly.
 * options - bit mapped.  See #define's below.
 * call_every_x_minutes - if non-zero, then the minimum number of days between calls
 *   to that system, even if there is nothing waiting to be sent there.
 * password - is the password used for connection to this system.
 */
#define unused_options_sendback 0x0001     /* & they can send data back */
#define unused_options_ATT_night 0x0002    /* - callout only at AT&T nigh hours */
#define unused_options_ppp 0x0004          /* _ transfer via PPP */
#define options_no_call 0x0008             /* + don't call that system, it will */
#define unused_options_receive_only 0x0010 /* ~ never send anything */
#define unused_options_once_per_day 0x0020 /* ! only call once per day */
#define unused_options_compress 0x0040     /* ; compress data */
#define unused_options_hslink 0x0080       /* ^ use HSLINK if available */
#define unused_options_force_ac 0x0100     /* $ force area code on dial */
#define unused_options_dial_ten 0x0200     /* * use ten digit dialing format */
#define options_hide_pend 0x0400           /* = hide in pending display */

#ifndef __MSDOS__

/**
 * Indicates this is the fake FTN outbound node.  This should
 * not be exposed to users unless required. It's an implementation detail.
 */
static constexpr int16_t FTN_FAKE_OUTBOUND_NODE = 32765;
static constexpr char FTN_FAKE_OUTBOUND_ADDRESS[] = "@32765";

/**
 * Indicates this is the fake FTN outbound node.  This should
 * not be exposed to users unless required. It's an implementation detail.
 */
static constexpr int16_t INTERNET_NEWS_FAKE_OUTBOUND_NODE = 32766;
static constexpr char INTERNET_NEWS_FAKE_OUTBOUND_ADDRESS[] = "@32766";

/**
 * Indicates this is the fake FTN outbound node.  This should
 * not be exposed to users unless required. It's an implementation detail.
 */
static constexpr int16_t INTERNET_EMAIL_FAKE_OUTBOUND_NODE = 32767;
static constexpr char INTERNET_EMAIL_FAKE_OUTBOUND_ADDRESS[] = "@32767";

/**
 * Used to indicate no node number in functions that return -1 when no
 * wwivnet node number is found.
 */
static constexpr uint16_t WWIVNET_NO_NODE = 65535;

enum class fido_packet_t { unset, type2_plus };
enum class fido_transport_t { unset, directory, binkp };
enum class fido_mailer_t { unset, flo, attach };
enum class fido_bundle_status_t : char {
  normal = 'f',
  crash = 'c',
  direct = 'd',
  hold = 'h',
  immediate = 'i',
  // Do not use for creating a packet.
  unknown = 'x'
};

// Remember to update the serialize function in networks_cereal.h and also
// the code (packet_config_for) in fido_callout.cpp when updating these.
/**
 * Fido specific per-packet settings.
 */
struct fido_packet_config_t {
  // Type of packet to create
  fido_packet_t packet_type = fido_packet_t::unset;
  // File extension to map to type defined in archivers.dat
  std::string compression_type;
  // Password to use in the packet.  It must be <= 8 chars.
  std::string packet_password;
  // Password to use for areafix requests. It must be <= 8 chars.
  std::string areafix_password;
  // Maxumim size of the bundles in bytes before a new one is created.
  int max_archive_size = 0;
  // Maxumim size of the packets in bytes before a new one is created.
  int max_packet_size = 0;
  // Status used for the bundles or packets to send.
  // i.e.: CRASH, IMMEDIATE, NORMAL.
  fido_bundle_status_t netmail_status = fido_bundle_status_t::normal;
};

/**
 * Contains the binkp session specific settings. This can come
 * from a fidonet nodelist, WWIVnet's binkp.net, or overrides
 * specified in the address settings.
 */
struct binkp_session_config_t {
  std::string host;
  int port = 0;
  std::string password;
};

/**
 * Minimal callout configuration
 */
struct network_callout_config_t {
  bool auto_callouts{false};
  int call_every_x_minutes{15};
  int min_k{0};
};

/**
 * Specific config for a fido node.
 */
struct fido_node_config_t {
  // Space separated list of nodes that route through this node.
  // i.e.: "1:*" will route all of zone 1 through this node.
  std::string routes;
  // Configuration for packet specific options.
  fido_packet_config_t packet_config;
  // BinkP session options.
  binkp_session_config_t binkp_config;
  // Automatic callout config
  network_callout_config_t callout_config;
};

// Remember to update the serialize function in networks_cereal.h when updating these.
/**
 * Fido specific per-network settings.
 */
struct fido_network_config_t {
  // Your FTN network address. (i.e. 1:100/123) [3d-5d accepted].
  std::string fido_address;
  // Your FTN nodelist base name (i.e. NODELiST)
  std::string nodelist_base;
  // Fidonet mailer type {FLO, Attach}.
  // Only FLO is even close to being supported.
  fido_mailer_t mailer_type;
  // Type of transport to use for this packet.
  fido_transport_t transport;
  // Inbound directory for packets
  std::string inbound_dir;
  // Inbound temporary directory.
  std::string temp_inbound_dir;
  // Outbound temporary directory.
  std::string temp_outbound_dir;
  // Outbound directory for packets
  std::string outbound_dir;
  // Location of FidoNet NetMail directory.
  // Note that this usually lives under your mailer's directory
  // if you are using an ATTACH style mailer.
  std::string netmail_dir;
  // Configuration for packet specific options.
  fido_packet_config_t packet_config;
  // Location to move bad packets
  std::string bad_packets_dir;
  // Default Origin line to use for this network.
  std::string origin_line;
};

enum class network_type_t : uint8_t { wwivnet = 0, ftn, internet, news };

/**
 * Internal structure for networks.dat or networks.json used by WWIV.
 * On disk it's persisted as net_networks_rec_disk.
 */
struct net_networks_rec {
  /* type of network */
  network_type_t type;
  /* network name */
  char name[16];
  /* directory for net data */
  std::string dir;
  /* system number */
  uint16_t sysnum;

  // Used by FIDOnet type nodes.
  fido_network_config_t fido;
};

#endif // __MSDOS__

#endif // __INCLUDED_NET_H__
