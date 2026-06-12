#pragma once

#include <cstdint>

class UUID{
public:
  UUID();
  UUID(uint64_t uuid);

  bool IsValid() const{
    return m_UUID != 0;
  }

  operator uint64_t() const{
    return m_UUID;
  }
private:
  uint64_t m_UUID;
};