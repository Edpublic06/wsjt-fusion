#ifndef JTTY_RECEIVE_HISTORY_HPP
#define JTTY_RECEIVE_HISTORY_HPP

#include <QtGlobal>
#include <algorithm>
#include <deque>
#include <vector>

namespace Jtty {
class ReceiveHistory
{
public:
  static constexpr qint64 capacity = 180 * 12000;
  struct Block {
    quint64 reception;
    qint64 first;
    std::vector<short> samples;
    qint64 end() const { return first + qint64(samples.size()); }
  };

  explicit ReceiveHistory(qint64 limit = capacity) : limit_(std::max<qint64>(1, limit)) {}

  bool append(quint64 reception, qint64 first, short const* samples, int count)
  {
    if (count <= 0 || first < 0 || count > limit_) return false;
    if (!blocks_.empty() && blocks_.back().reception == reception
        && blocks_.back().end() != first) return false;
    blocks_.push_back({reception, first, {samples, samples + count}});
    size_ += count;
    while (size_ > limit_) {
      auto& front = blocks_.front();
      auto const remove = std::min(size_ - limit_, qint64(front.samples.size()));
      if (remove == qint64(front.samples.size())) blocks_.pop_front();
      else {
        front.samples.erase(front.samples.begin(), front.samples.begin() + remove);
        front.first += remove;
      }
      size_ -= remove;
    }
    return true;
  }

  std::vector<short> snapshot(quint64 reception, qint64 first, qint64 end) const
  {
    std::vector<short> result;
    if (first < 0 || end <= first || end - first > limit_) return result;
    result.reserve(end - first);
    qint64 next = first;
    for (auto const& block : blocks_) {
      if (block.reception != reception || block.end() <= next) continue;
      if (block.first > next) return {};
      auto const stop = std::min(end, block.end());
      result.insert(result.end(), block.samples.begin() + (next - block.first),
                    block.samples.begin() + (stop - block.first));
      next = stop;
      if (next == end) return result;
    }
    return {};
  }

  qint64 first(quint64 reception) const
  {
    for (auto const& block : blocks_) if (block.reception == reception) return block.first;
    return -1;
  }
  qint64 end(quint64 reception) const
  {
    for (auto it = blocks_.rbegin(); it != blocks_.rend(); ++it)
      if (it->reception == reception) return it->end();
    return -1;
  }
  qint64 size() const { return size_; }
  void clear() { blocks_.clear(); size_ = 0; }

private:
  qint64 limit_;
  qint64 size_ = 0;
  std::deque<Block> blocks_;
};
}
#endif
