require 'tty-prompt'
require 'optparse'
require 'io/console'
require 'json'

$SPECTRE_PATH = "./build/spectre"
$JSON_PATH = File.expand_path "~/.passe.json"

`echo "{}" > #{$JSON_PATH}` unless File.exists? $JSON_PATH

data = File.read $JSON_PATH
$json = JSON.parse data

def bail msg=nil
    puts "ERROR: #{msg}" unless msg.nil?
    abort
end

def selectUser
    bail "No users, please run `passe ...`" if $json.keys.empty?
    return $json.keys[0] if $json.keys.length == 1
    prompt = TTY::Prompt.new
    prompt.select "Select a user:", $json.keys, filter: true, cycle: true
end

def selectSite user
    bail "No sites saved, please run `passe ...`" if not $json.has_key? user or $json[user].empty?
    prompt = TTY::Prompt.new
    prompt.select "Select a site (#{user}):", $json[user], filter: true, cycle: true
end

def getMasterPassword
    puts "Enter Master Password:"
    STDIN.noecho(&:gets).chomp
end

case $*[0]
when "user"
    bail "No username passed to `user` option" if $*[1].nil?
    bail "Username already exists" if $json.has_key? $*[1]
    puts "Create user `#{$*[1]}`"
    $json[$*[1]] = []
when "add"
    bail "No username supplied" if $*[1].nil?
    bail "No site identifier supplied" if $*[2].nil?
    bail "Username `#{$*[1]}` doesn't exist" unless $json.has_key? $*[1]
    bail "Site `#{$*[2]}` already exists for `#{$*[1]}`" if $json[$*[1]].include? $*[2]
    puts "Added `#{$*[2]}` to `#{$*[1]}`"
    $json[$*[1]].append $*[2]
when "del"
    bail "No username supplied" if $*[1].nil?
    bail "No site identifier supplied" if $*[2].nil?
    bail "Username `#{$*[1]}` doesn't exist" unless $json.has_key? $*[1]
    bail "Site `#{$*[2]}` doesn't exist for `#{$*[1]}`" unless $json[$*[1]].include? $*[2]
    puts "Delete `#{$*[2]}` from `#{$*[1]}`"
    $json[$*[1]].delete($*[2])
when "generate", "gen"
    puts `#{$SPECTRE_PATH} #{$*.drop(1).join(' ')}`
else
    if $*[0]
        bail "Invalid mode `#{$*[0]}`"
    else
        user = selectUser
        site = selectSite user
        pass = getMasterPassword
        puts `#{$SPECTRE_PATH} --name "#{user}" --pass "#{pass}" --site "#{site}"`
    end
end

File.open($JSON_PATH, 'w') do |f|
    f.write $json.to_json
end
