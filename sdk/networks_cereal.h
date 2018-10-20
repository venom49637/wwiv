/**************************************************************************/
/*                                                                        */
/*                          WWIV Version 5.x                              */
/*             Copyright (C)2014-2017, WWIV Software Services             */
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
/**************************************************************************/
#ifndef __INCLUDED_SDK_NETWORKS_CEREAL_H__
#define __INCLUDED_SDK_NETWORKS_CEREAL_H__

// We want to override how we store network_type_t to store it as a string, not int.
// This has to be in the global namespace. 
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(network_type_t, cereal::specialization::non_member_load_save_minimal);
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(fido_packet_t, cereal::specialization::non_member_load_save_minimal);
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(fido_bundle_status_t, cereal::specialization::non_member_load_save_minimal);

namespace cereal {

#define SERIALIZE(n, field) { try { ar(cereal::make_nvp(#field, n.field)); } catch(const cereal::Exception&) { ar.setNextName(nullptr); } }

template <typename T> inline
std::string to_enum_string(const T& t, const std::vector<std::string>& names) {
  try {
    return names.at(static_cast<size_t>(t));
  } catch (std::out_of_range&) {
    return names.at(0);
  }
}

template <typename T> inline
T from_enum_string(const std::string& v, const std::vector<std::string>& names) {
  try {
    for (size_t i = 0; i < names.size(); i++) {
      if (v == names.at(i)) {
        return static_cast<T>(i);
      }
    }
  } catch (std::out_of_range&) {
    // NOP
  }
  return static_cast<T>(0);
}

template <class Archive> inline
std::string save_minimal(Archive const &, const network_type_t& t) {
  return to_enum_string<network_type_t>(t, {"wwivnet", "fido", "internet"});
}
template <class Archive> inline
void load_minimal(Archive const &, network_type_t& t, const std::string& v) {
  t = from_enum_string<network_type_t>(v, {"wwivnet", "fido", "internet"});
}

template <class Archive> inline
std::string save_minimal(Archive const &, const fido_bundle_status_t& t) {
  auto c = static_cast<char>(t);
  return std::string(1, c);
}
template <class Archive> inline
void load_minimal(Archive const &, fido_bundle_status_t& t, const std::string& v) {
  char c = 'f';
  if (!v.empty()) {
    c = v.front();
  }
  t = static_cast<fido_bundle_status_t>(c);
}

template <class Archive> inline
std::string save_minimal(Archive const &, const fido_packet_t& t) {
  return to_enum_string<fido_packet_t>(t, {"unset", "type2+"});
}
template <class Archive> inline
void load_minimal(Archive const &, fido_packet_t& t, const std::string& v) {
  t = from_enum_string<fido_packet_t>(v, {"unset", "type2+"});
}

template <class Archive> inline
std::string save_minimal(Archive const &, const fido_transport_t& t) {
  return to_enum_string<fido_transport_t>(t, {"unset", "directory", "binkp"});
}
template <class Archive> inline
void load_minimal(Archive const &, fido_transport_t& t, const std::string& v) {
  t = from_enum_string<fido_transport_t>(v, {"unset", "directory", "binkp"});
}

template <class Archive> inline
std::string save_minimal(Archive const &, const fido_mailer_t& t) {
  return to_enum_string<fido_mailer_t>(t, {"unset", "flo", "attach"});
}
template <class Archive> inline
void load_minimal(Archive const &, fido_mailer_t& t, const std::string& v) {
  t = from_enum_string<fido_mailer_t>(v, {"unset", "flo", "attach"});
}

template <class Archive>
void serialize(Archive & ar, fido_packet_config_t& n) {
  SERIALIZE(n, packet_type);
  SERIALIZE(n, compression_type);
  SERIALIZE(n, packet_password);
  SERIALIZE(n, areafix_password);
  SERIALIZE(n, max_archive_size);
  SERIALIZE(n, max_packet_size);
  SERIALIZE(n, netmail_status);
}

// fido_node_config_t
template <class Archive>
void serialize(Archive & ar, fido_node_config_t& n) {
  SERIALIZE(n, routes);
  SERIALIZE(n, packet_config);
  SERIALIZE(n, binkp_config);
  SERIALIZE(n, callout_config);
}

// binkp_session_config_t
template <class Archive>
void serialize(Archive & ar, binkp_session_config_t& n) {
  SERIALIZE(n, host);
  SERIALIZE(n, port);
  SERIALIZE(n, password);
}

// network_callout_config_t
template <class Archive> void serialize(Archive& ar, network_callout_config_t& n) {
  SERIALIZE(n, auto_callouts);
  SERIALIZE(n, call_every_x_minutes);
  SERIALIZE(n, min_k);
}

template <class Archive>
void serialize(Archive & ar, fido_network_config_t& n) {
  SERIALIZE(n, fido_address);
  SERIALIZE(n, nodelist_base);
  SERIALIZE(n, mailer_type);
  SERIALIZE(n, transport);
  SERIALIZE(n, inbound_dir);
  SERIALIZE(n, temp_inbound_dir);
  SERIALIZE(n, temp_outbound_dir);
  SERIALIZE(n, outbound_dir);
  SERIALIZE(n, netmail_dir);
  SERIALIZE(n, bad_packets_dir);
  SERIALIZE(n, origin_line);
  SERIALIZE(n, packet_config);
}

// This has to be in the cereal or default to match net_networks_rec which
// is in the default namespace.
template <class Archive>
void serialize(Archive & ar, net_networks_rec& n) {
  SERIALIZE(n, type);
  try {
    std::string name(n.name);
    ar(make_nvp("name", name));
    to_char_array(n.name, name);
  } catch (const cereal::Exception&) {
    ar.setNextName(nullptr);
  }
  SERIALIZE(n, dir);
  SERIALIZE(n, sysnum);

  // Seralize the Fido config
  if (n.type == network_type_t::ftn) {
    SERIALIZE(n, fido);
  }
}

}  // namespace cereal



#endif  // __INCLUDED_SDK_NETWORKS_CEREAL_H__
