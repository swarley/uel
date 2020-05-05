
Gem::Specification.new do |spec|
  spec.name          = "UEL"
  spec.version       = "0.1.0"
  spec.authors       = ["Matthew Carey"]
  spec.email         = ["matthew.b.carey@gmail.com"]

  spec.summary       = %q{untitled etf lib}
  spec.description   = %q{untitled etf lib}
  spec.homepage      = "https://github.com/swarley/uel"
  spec.license       = "MIT"
  spec.required_ruby_version = Gem::Requirement.new(">= 2.3.0")

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = spec.homepage
  spec.metadata["changelog_uri"] = spec.homepage

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files         = Dir.chdir(File.expand_path('..', __FILE__)) do
    `git ls-files -z`.split("\x0").reject { |f| f.match(%r{^(test|spec|features)/}) }
  end
  spec.extensions    << "ext/uel/extconf.rb"
  spec.bindir        = "exe"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]

  spec.add_development_dependency "rake-compiler"
end
