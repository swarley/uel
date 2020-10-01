### Please use [vox-etf](https://github.com/swarley/vox-etf) if possible, as it's more stable.

# UEL (Untitled ETF Lib)

The other ETF lib I could find is pretty slow, so here's this one. I couldn't think of a good name.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'uel'
```

And then execute:

    $ bundle install

Or install it yourself as:

    $ gem install uel

## Usage

```ruby
require 'uel'

UEL.encode(some_object)
UEL.decode(some_string_of_etf_data)
```

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/swarley/uel. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [code of conduct](https://github.com/swarley/uel/blob/master/CODE_OF_CONDUCT.md).


## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).

## Code of Conduct

Everyone interacting in the UEL project's codebases, issue trackers, chat rooms and mailing lists is expected to follow the [code of conduct](https://github.com/swarley/uel/blob/master/CODE_OF_CONDUCT.md).
