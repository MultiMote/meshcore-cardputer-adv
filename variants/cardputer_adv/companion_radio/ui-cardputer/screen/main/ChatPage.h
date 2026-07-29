#pragma once

#include "../CardputerScreen.h"
#include "../MainScreen.h"
#include "companion_radio/CardputerMesh.h"

class ChatPage : public CardputerScreen {
private:
  MainScreen *_p;
  String chat_text_box;
  String last_sent_message;

  ChannelDetails current_channel;
  int current_channel_idx = -1;

  ContactInfo current_contact;
  int current_contact_idx = -1;

  ChatHistory chat_history;
  int chat_history_offset = 0; // from bottom

  void sendChatMessage();

public:
  ChatPage(MainScreen *parent) : _p(parent) { chat_text_box.reserve(MAX_MESSAGE_LENGTH); };
  int render(CardputerDisplay &lcd) override;
  bool handleInput(Keyboard::Event &e) override;
  void selectContact(const ContactInfo &contact, int idx);
  void selectChannel(const ChannelDetails &channel, int idx);

  bool onChannelMessageRecv(const mesh::GroupChannel &channel, const char *text);
  bool onContactMessageRecv(const ContactInfo &contact, const char *text);

  void refreshSelectedContact();
};
