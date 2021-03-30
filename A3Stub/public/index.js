// Put all onload AJAX calls here, and event listeners

let currentFileName = "";

$(document).ready(function() {
    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/filelogdata',   //The server endpoint we are connecting to
        data: {
            stuff: "Value 1",
            junk: "Value 2"
        },
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            //$('#blah').html("On page load, received string '"+data.stuff+"' from server");
            //We write the object to the console to show that the request was successful
            console.log(data); 
            var jsonArray = JSON.parse(data.jsondata);
            for (var json of jsonArray){
                $('#fileLogPanel').find('tbody').append(JSONtoFileLogPanel(json));
            }

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("On page load, received error from server");
            console.log(error); 
        }
    });

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    $('#someform').submit(function(e){
        $('#blah').html("Form has data: "+$('#entryBox').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });
    $.ajax({
        type : 'get',
        url : '/getFilenames',
        dataType : 'json',
        success : function(data){
            var array = JSON.parse(data.filenames);
            for (var filename of array){
                console.log(filename);
                $('.dropdown-content').append(JSONtoDropdown(filename));
            }
        }
    });
    $(document).on('click', '.dropdownLink', function(){
        currentFileName = $(this).text().toString();
        $.ajax({
            type : 'get',
            url : '/getGPXViewPanel',
            dataType : 'json',
            data : {
                filename : currentFileName
            },
            success : function(data){
                //Append data to gpxviewtable
            },
            fail : function(data){
                //Append failed to gpxviewtable
            }
        });
    });
});

//End of document.ready

//Helper functions:

function JSONtoFileLogPanel(json){
    var obj = JSON.parse(json);
    var fn = obj.fn.split("/");
    var html = "<tr>"+"<td>"+"<a href = \"/uploads/\">"+
            fn[fn.length-1]+"</a>"+"</td>"+
            "<td>"+obj.version+"</td>"
            +"<td>"+obj.creator+"</td>"+"<td>"+
            obj.numWaypoints+"</td>"+"<td>"+
            obj.numRoutes+"</td>"+"<td>"+
            obj.numTracks+"</td>"+"</tr>";
    return html;
}
/*<div class="dropdown-content">
    <a href="#">Link 1</a>
    <a href="#">Link 2</a>
    <a href="#">Link 3</a>
</div>*/

function JSONtoDropdown(json){
    var html = "<a href=\"#\" class=\"dropdownLink\">"+json+"</a>";
    return html;
}
